#ifndef PARSING_H
#define PARSING_H

ircLine_t *parseLine (char *line);
ircLine_t *parseLineWithoutColon (char *line);
void parseMask (ircLine_t *ircLine);

#endif
