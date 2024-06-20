# artadobot
This crawler requires `libcurl` and `libxml2` to run

## Configuring
Edit the `config.hpp` file according to your preferences.

| Option | Description |
| ------ | ----------- |
| `CRAWLER_NAME` | Your crawler's name that will be shown in the user-agent. |

## Compiling
```
cmake .
make
```

## Testing
```
./crawler <url> <depth>
```

## Debug Options
These options can be set in `main.cpp` on the line `debug_level = 0;` by or'ing the options.
| Option | Bit | Description |
| ------ | --- | ----------- |
| `DBG_CONT` | 1 | Prints out the contents of the visited page |
| `DBG_RECR` | 2 | Prints out the elements of the visited page in nested format |
| `DBG_INFO` | 3 | Prints out the collected information of the visited pages |
| `DBG_QUEUE` | 4 | Prints out the new queue to be used by the next iteration (next depth) |

## To do

- [x] Scraping a page
- [x] Check robots.txt
- [ ] Save page data
- [x] Recursively browse pages
- [ ] Save queue
- [ ] Optimizations
  - [x] Cache robots.txt files for websites
  - [ ] Save visit time for websites and check visit time of pages to not visit the same pages for 30 days