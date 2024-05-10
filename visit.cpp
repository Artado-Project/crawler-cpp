#include <iostream>
#include <string>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include "core.hpp"

#define EQCS(str1, str2) (!xmlStrcasecmp((xmlChar *)str1, (xmlChar *)str2))

#define DBG_CONT 1
#define DBG_RECR 2
#define DBG_INFO 4

using elfunc = void(*)(struct site_info*, xmlNodePtr);

uint16_t debug_level = DBG_INFO;

#pragma region utilities
std::string get_base_url(const std::string& full_url) {
    size_t scheme_end = full_url.find("://");
    if (scheme_end == std::string::npos) {
        // if there is no scheme, assume it might still be a URL starting directly with a domain
        scheme_end = 0;
    } else {
        scheme_end += 3;
    }

    size_t base_end = full_url.find_first_of("/?#", scheme_end);
    if (base_end == std::string::npos) {
        return full_url;
    }

    return full_url.substr(0, base_end);
}

bool is_absolute_url(const std::string& url) {
    return url.find("://") != std::string::npos || url.starts_with("//");
}

std::string make_absolute_url(const std::string& base_url, const std::string& relative_url) {
    if (relative_url.starts_with("/")) {
        return get_base_url(base_url) + relative_url;
    } else {
        return base_url + '/' + relative_url;
    }
}

void trim(std::string &str){
    int i=0;

    while (isspace(str[i])!=0)
        i++;
    str = str.substr(i,str.length()-i);

    i=str.length()-1;
    while (isspace(str[i])!=0)
        i--;
    str = str.substr(0,i+1);
}

// callback function for libcurl to write received data
size_t write_data(void *contents, size_t size, size_t nmemb, std::string *data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}
#pragma endregion
#pragma region element handlers
void element_title(struct site_info *siteinfo, xmlNodePtr cn) {
    xmlNodePtr contentNode = cn->children;
    while (contentNode != nullptr) {
        if (contentNode->type == XML_TEXT_NODE) {
            std::string title = (char*)contentNode->content;
            trim(title);
            siteinfo->title = title;
            break;
        }
        contentNode = contentNode->next;
    }
}

void element_meta(struct site_info *siteinfo, xmlNodePtr cn) {
    xmlChar* name_attr = xmlGetProp(cn, (const xmlChar *)"name");
    xmlChar* content_attr = xmlGetProp(cn, (const xmlChar *)"content");

    if (name_attr && content_attr) {
        if (EQCS(name_attr, "description")) { // description
            std::string content = (char *)content_attr;
            trim(content);
            siteinfo->description = content;
        } else if (EQCS(name_attr, "keywords")) { // keywords
            std::istringstream keywordStream((char *)content_attr);
            std::string keyword;
            while (getline(keywordStream, keyword, ',')) {
                trim(keyword);
                siteinfo->keywords.push_back(keyword);
            }
        }
    }

    xmlChar* http_equiv_attr = xmlGetProp(cn, (const xmlChar *)"http-equiv");
    if (http_equiv_attr) {
        if (EQCS(http_equiv_attr, "content-language") && content_attr) { // language
            siteinfo->language = (char*)content_attr;
        }
    }

    xmlFree(name_attr);
    xmlFree(content_attr);
    xmlFree(http_equiv_attr);
}

void element_link(struct site_info *siteinfo, xmlNodePtr cn) {
    xmlChar* rel_attr = xmlGetProp(cn, (const xmlChar*)"rel");
    xmlChar* href_attr = xmlGetProp(cn, (const xmlChar*)"href");
    xmlChar* sizes_attr = xmlGetProp(cn, (const xmlChar*)"sizes");
    xmlChar* type_attr = xmlGetProp(cn, (const xmlChar*)"type");

    if (href_attr) {
        std::string href = (char *)href_attr;

        if (rel_attr) {
            if (EQCS(rel_attr, "icon") || EQCS(rel_attr, "shortcut icon") || EQCS(rel_attr, "apple-touch-icon")) {
                struct site_icon icon;
                icon.src = href;

                if (sizes_attr) {
                    icon.size = (char *)sizes_attr;
                }

                if (type_attr) {
                    icon.type = (char *)type_attr;
                }

                siteinfo->icons.push_back(icon);
            }
        } else {
            siteinfo->links.push_back(href);
        }
    }

    // Clean up
    xmlFree(rel_attr);
    xmlFree(href_attr);
    xmlFree(sizes_attr);
    xmlFree(type_attr);
}

void element_a(struct site_info *siteinfo, xmlNodePtr cn) {
    xmlChar* href_attr = xmlGetProp(cn, (const xmlChar*)"href");
    xmlChar* rel_attr = xmlGetProp(cn, (const xmlChar*)"rel");

    if (href_attr) {
        bool is_nofollow = false;
        if (rel_attr) {
            std::string rel_value = (char*)rel_attr;
            // transform to lowercase for the func find
            std::transform(rel_value.begin(), rel_value.end(), rel_value.begin(), ::tolower);
            is_nofollow = rel_value.find("nofollow") != std::string::npos;
        }

        if (!is_nofollow) {
            std::string url = (char*)href_attr;
            if (!is_absolute_url(url)) {
                url = make_absolute_url(siteinfo->url, url);
            }
            siteinfo->links.push_back(url);
        }

        xmlFree(href_attr);
        if (rel_attr) {
            xmlFree(rel_attr);
        }
    }
}

std::map<std::string, elfunc> funcMap = {
    {"title", &element_title},
    {"meta", &element_meta},
    {"link", &element_link},
    {"a", &element_a},
};
#pragma endregion
#pragma region base node handler
void process_node(struct site_info* siteinfo, xmlNodePtr node, int indent) {
    if (node == NULL) return;

    xmlChar* lang_attr = xmlGetProp(node, (const xmlChar *)"lang");
    if(indent == 0 && lang_attr)
    {
        siteinfo->language = (char*)lang_attr;
    }

    if (node->type == XML_ELEMENT_NODE) {
        if (debug_level & DBG_RECR)
        {
            for(int i = 0; i < indent; i++)
                std::cout << "  ";
            std::cout << "el: " << node->name << std::endl;
        }
        std::string el_name = (char*)node->name;
        if (funcMap.find(el_name) != funcMap.end()) {
            elfunc f = funcMap[el_name];
            (*f)(siteinfo, node);
        }
    }

    process_node(siteinfo, node->children, indent + 1);
    process_node(siteinfo, node->next, indent);

    xmlFree(lang_attr);
}
#pragma endregion
#pragma region visit page
struct site_info visit_page(std::string url)
{
    struct site_info ret_value;
    ret_value.url = url;

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        std::string content;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            if (debug_level & DBG_CONT)
            {
                std::cout << content << std::endl;
            }

            htmlDocPtr doc = htmlReadMemory(content.c_str(), content.size(), NULL, NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
            xmlNodePtr root = xmlDocGetRootElement(doc);
            process_node(&ret_value, root, 0);
            xmlFreeDoc(doc);
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        exit(1);
    }
    curl_global_cleanup();

    if (debug_level & DBG_INFO)
    {
        std::cout << "Title: " << ret_value.title << std::endl;
        std::cout << "Description: " << ret_value.description << std::endl;
        std::cout << "Language: " << ret_value.language << std::endl;
        std::cout << "Keywords: ";
        for(auto keyword : ret_value.keywords)
        {
            std::cout << keyword << " ";
        }
        std::cout << std::endl;
        std::cout << "Icons: " << std::endl;
        for(auto icon : ret_value.icons)
        {
            std::cout << "  Icon: " << std::endl;
            std::cout << "    Link: " << icon.src << std::endl;
            std::cout << "    Size: " << icon.size << std::endl;
            std::cout << "    Type: " << icon.type << std::endl;
        }
        std::cout << "Links: " << std::endl;
        for(auto link : ret_value.links)
        {
            std::cout << "  " << link << std::endl;
        }
    }
    return ret_value;
}
#pragma endregion
