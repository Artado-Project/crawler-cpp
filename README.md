# artadobot
A crawler made for Artado Search.

## Configuring
Edit the `config.hpp` file according to your preferences.

| Option | Description |
| ------ | ----------- |
| `CRAWLER_NAME` | Your crawler's name that will appear in the user-agent. |

## Compiling
This crawler requires `libcurl` and `libxml2` to run
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

## License
Copyright (C) 2024  Çınar Yılmaz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.