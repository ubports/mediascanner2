/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include<vector>
#include<string>
#include<fstream>
#include<cstdio>
#include<random>

using namespace std;

#define RNDWORD words[rnd() % words.size()]

int main() {
    ifstream ifile("/usr/share/dict/words");
    string line;
    vector<string> words;
    random_device rnd;
    while(getline(ifile, line)) {
        if(line.length() >=3) {
            if(line[line.length()-2] == '\'')
                continue;
        }
        words.push_back(line);
    }
    for(int artistCount=0; artistCount < 1000; artistCount++) {
        string artist = RNDWORD + " " + RNDWORD;
        for(int albumCount=0; albumCount < 3; albumCount++) {
            string album = RNDWORD + " " + RNDWORD + " " + RNDWORD;
            for(int trackCount=0; trackCount < 10; trackCount++) {
                int numWords = rnd() % 4 + 1;
                string track(RNDWORD);
                for(int i=1; i<numWords; i++) {
                    track += " " + RNDWORD;
                }
                printf("%s, %s, %s\n", artist.c_str(), album.c_str(), track.c_str());
            }
        }
    }
    return 0;
}
