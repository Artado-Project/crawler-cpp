#include <iostream>
#include <string>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include "core.hpp"

#define EQCS(str1, str2) (!xmlStrcasecmp((xmlChar *)str1, (xmlChar *)str2))

using elfunc = void(*)(struct site_info*, xmlNodePtr);

#pragma region utilities
bool startswithcase(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), str.begin(), [](char a, char b) {
        return std::tolower(a) == std::tolower(b);
    });
}

std::string get_base_url(const std::string& full_url) {
    size_t scheme_end = full_url.find("://");
    if (scheme_end == std::string::npos) {
        scheme_end = 0;
    } else {
        scheme_end += 3;
    }

    size_t base_end = full_url.find_first_of("/?#", scheme_end);

    return full_url.substr(scheme_end, base_end);
}

std::string get_base_url_scheme(const std::string& full_url) {
    size_t scheme_end = full_url.find("://");
    if (scheme_end == std::string::npos) {
        scheme_end = 0;
    } else {
        scheme_end += 3;
    }

    size_t base_end = full_url.find_first_of("/?#", scheme_end);

    return full_url.substr(0, base_end);
}

bool is_absolute_url(const std::string& url) {
    return url.find("://") != std::string::npos || url.starts_with("//");
}

std::string make_absolute_url(const std::string& base_url, const std::string& relative_url) {
    if (relative_url.starts_with("/")) {
        return get_base_url_scheme(base_url) + relative_url;
    } else {
        return base_url + '/' + relative_url;
    }
}

// This has to be strict to make sure there aren't duplicate URLs.
std::string get_normalized_url(const std::string& base_url, const std::string& full_url) {
    std::string normalized_url = full_url;
    if (!is_absolute_url(normalized_url)) {
        normalized_url = make_absolute_url(base_url, normalized_url);
    }
    size_t base_end = normalized_url.find_first_of("#", 0);

    normalized_url = normalized_url.substr(0, base_end);
    if (!normalized_url.empty() && normalized_url.back() != '/') {
        normalized_url += '/';
    }

    return normalized_url;
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

// Callback function for libcurl to write received data
size_t write_data(void *contents, size_t size, size_t nmemb, std::string *data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

size_t write_data_file(void *contents, size_t size, size_t nmemb, FILE *file) {
    size_t written = fwrite(contents, size, nmemb, file);
    return written;
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
        if (EQCS(name_attr, "description")) {
            std::string content = (char *)content_attr;
            trim(content);
            siteinfo->description = content;
        } else if (EQCS(name_attr, "keywords")) {
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
        if (EQCS(http_equiv_attr, "content-language") && content_attr) {
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
            siteinfo->links.insert(get_normalized_url(siteinfo->url, href));
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
            // Transform to lowercase for .find()
            std::transform(rel_value.begin(), rel_value.end(), rel_value.begin(), ::tolower);
            is_nofollow = rel_value.find("nofollow") != std::string::npos;
        }

        if (!is_nofollow) {
            std::string url = (char*)href_attr;
            siteinfo->links.insert(get_normalized_url(siteinfo->url, url));
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
#pragma region robots.txt
struct robots_txt visit_robotstxt(CURL *curl, std::string robotstxt_url)
{
    struct robots_txt ret_value;

    std::string cache_file_path = "./.artadobot_cache/robots/" + get_base_url(robotstxt_url) + ".txt";

    if (!std::filesystem::exists(cache_file_path))
    {
        FILE* cache_file = fopen(cache_file_path.c_str(), "w");
        if (!cache_file) {
            std::cerr << "Failed to open cache file " << "./.artadobot_cache/robots/" + get_base_url(robotstxt_url) + ".txt" << std::endl;
            return ret_value;
        }

        curl_easy_setopt(curl, CURLOPT_URL, robotstxt_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_file);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, cache_file);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "ArtadoBot/0.1");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);

        fclose(cache_file);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return ret_value;
        }
        long http_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code != 200)
        {
            std::cerr << "Status for " << robotstxt_url << ": " << http_code << " (not 200 OK)" << std::endl;
            return ret_value;
        }
    }

    ret_value.exists = true;
    std::ifstream robots(cache_file_path);
    std::string line;
    bool ua_match = false;
    while (getline(robots, line)) {
        trim(line);
        if (startswithcase(line, "user-agent:"))
        {
            ua_match = false;
            std::string ua = line.substr(11);
            trim(ua);
            if (EQCS(ua.c_str(), "artadobot") || ua == "*")
                ua_match = true;
        }
        else if (ua_match && startswithcase(line, "disallow:"))
        {
            std::string path = line.substr(9);
            trim(path);
            if (path != "")
            {
                ret_value.disallowed_urls.push_back(make_absolute_url(get_base_url(robotstxt_url), path));
            }
        }
    }
    return ret_value;
}

bool robots_allowed(struct site_info *siteinfo, std::string url)
{
    for (std::string disurl : siteinfo->robots.disallowed_urls)
    {
        if (startswithcase(url, disurl))
            return false;
    }
    return true;
}
#pragma endregion
#pragma region visit page
struct site_info visit_page(std::string url)
{
    struct site_info ret_value;
    ret_value.url = url;
    ret_value.visit_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();

    if (curl) {
        ret_value.robots = visit_robotstxt(curl, get_base_url(url) + "/robots.txt");
        if (!robots_allowed(&ret_value, url))
        {
            std::cerr << "ArtadoBot is not allowed at " << url << " address." << std::endl;
            ret_value.status = -0xA97AD0;
            return ret_value;
        }

        std::string content;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "ArtadoBot/0.1");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ret_value.status);
            if (ret_value.status != 200)
            {
                std::cerr << "Status for " << url << ": " << ret_value.status << " (not 200 OK)" << std::endl;
                return ret_value;
            }
            if (debug_level & DBG_CONT)
                std::cout << content << std::endl;

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
    return ret_value;
}
#pragma endregion
