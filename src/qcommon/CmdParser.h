/*
===========================================================================
Copyright (C) 2018 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <iostream>
#include <vector>

using Args = std::vector<std::string>;

struct Parser {

    Args args;

    Args& Parse(std::string src)
    {
        args.clear();
        token.clear();
        state = prev_state = START;
        scan = SCAN_START;

        for (int ch : src)
        {
            if (!process(ch))
                continue;

            args.emplace_back(token);
            token.clear();
        }

        // flush any partial states
        process(-1);

        if (token.size())
            args.emplace_back(token);

        return args;
    }

private:

    enum State {
        START,
        DQUOTE,
        ESCAPE,
        COMMENT_START,
        ONELINE_COMMENT
    };

    enum ScanState {
        SCAN_START,
        SCAN_FOUND,
        SCAN_BREAK,
        SCAN_STOP
    };

    bool process(int ch)
    {
        if (ch == -1)
        {
            if (state == COMMENT_START)
                token += '/';
            else if (state == ESCAPE)
                token += '\\';
            else
                return false;
            return true;
        }

        switch (state) {
again:
            case START:
                if (isspace(ch)) {
                    if (scan == SCAN_FOUND)
                        scan = SCAN_BREAK;
                    break;
                }
                else if (ch == '"') {
                    state = DQUOTE;
                    break;
                }
                else if (ch == '/') {
                    state = COMMENT_START;
                    break;
                }
                else if (ch == '\\') {
                    prev_state = state;
                    state = ESCAPE;
                    break;
                }

                scan = SCAN_FOUND;
                token += ch;
                break;

            // Begin comment scanner
            case COMMENT_START:
                if (ch == '/') {
                    scan = SCAN_STOP;
                    state = ONELINE_COMMENT;
                    break;
                }

                token += '/';
                state = START;
                scan = SCAN_FOUND;
                goto again;

            // Oneline comment ends the processing. We could optimize this
            case ONELINE_COMMENT:
                break;

            // String literal
            case DQUOTE:
                if (ch == '\\') {
                    prev_state = state;
                    state = ESCAPE;
                    break;
                }
                else if (ch == '"') {
                    state = START;
                    scan = SCAN_BREAK;
                    break;
                } 

                token += ch;
                break;

            case ESCAPE:
                token += ch;
                state = prev_state;
                break;
        }

        if (scan == SCAN_BREAK) {
            scan = SCAN_START;
            state = START;
            return true;
        }

        if (scan == SCAN_STOP) {
            if (!token.empty())
                return true;
            return false;
        }

        return false;
    }

    State state;
    State prev_state; // used to restore state after processing an escaped literal char. 
    ScanState scan;
    std::string token;
};
