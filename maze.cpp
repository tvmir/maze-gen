#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <cstdlib>

enum mvmnt {
    GRID,
    GENERATE,
    CALCULATE,
    VISITING,
    VISITED
};

int WIDTH = 600;
int HEIGHT = 600;
int row = 60;
int col = 60;   

int state = GRID;
bool generated = false;
bool visualized = false;

class Spot: public sf::Drawable {
    private:
        sf::Color color = sf::Color::White;
    
    public:
        float top = 0;
        float down = 0;
        float dir = 0;
        float x = 0;
        float y = 0;
        bool visited = false;

        std::vector<bool> walls = {true, true, true, true};
        std::vector<Spot*> adjoin;
        Spot* prev = 0;

        void generateMaze(std::vector<std::vector<Spot>> &cell) {
            adjoin.clear();
            if (x<col-1 && !walls[1])
                adjoin.push_back(&cell[x+1][y]);
            if (x>0 && !walls[3])
                adjoin.push_back(&cell[x-1][y]);
            if (y<row-1 && !walls[2])
                adjoin.push_back(&cell[x][y+1]);
            if (x>0 && !walls[0])
                adjoin.push_back(&cell[x][y-1]);
        }

        void addAdjoin(std::vector<std::vector<Spot>> &cell) {
            adjoin.clear();
            if (x<col-1) {
                if (!cell[x+1][y].visited)
                    adjoin.push_back(&cell[x+1][y]);
            }
            if (x>0) {
                if (!cell[x-1][y].visited)
                    adjoin.push_back(&cell[x-1][y]);
            }
            if (y<row-1) {
                if (!cell[x][y+1].visited)
                    adjoin.push_back(&cell[x][y+1]);
            }
            if (y>0) {
                if (!cell[x][y-1].visited)
                    adjoin.push_back(&cell[x][y-1]);
            }
        }

        void setColor(sf::Color updateColor) {
            color = updateColor;
        }

        void coordinates(int x, int y) {
            this -> x = x;
            this -> y = y;
        }
        
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
            int cellWidth = WIDTH/col;
            int cellHeight = HEIGHT/row;
            sf::RectangleShape s(sf::Vector2f(cellWidth, cellHeight));
            s.move(sf::Vector2f(x*cellWidth, y*cellHeight));
            s.setFillColor(color);
            target.draw(s);

            if (walls[0]) {
                sf::VertexArray wall(sf::Lines, 2);
                wall[0].color = sf::Color::Black;
                wall[0].position = sf::Vector2f(x*cellWidth, y*cellHeight);
                wall[1].color = sf::Color::Black;
                wall[1].position = sf::Vector2f((x+1)*cellWidth, y*cellHeight);
                target.draw(wall);
            }
            if (walls[1]) {
                sf::VertexArray wall(sf::Lines, 2);
                wall[0].color = sf::Color::Black;
                wall[0].position = sf::Vector2f((x+1)*cellWidth, y*cellHeight);
                wall[1].color = sf::Color::Black;
                wall[1].position = sf::Vector2f((x+1)*cellWidth, (y+1)*cellHeight);
                target.draw(wall);
            }
            if (walls[2]) {
                sf::VertexArray wall(sf::Lines, 2);
                wall[0].color = sf::Color::Black;
                wall[0].position = sf::Vector2f(x*cellWidth, (y+1)*cellHeight);
                wall[1].color = sf::Color::Black;
                wall[1].position = sf::Vector2f((x+1)*cellWidth, (y+1)*cellHeight);
                target.draw(wall);
            }
            if (walls[3]) {
                sf::VertexArray wall(sf::Lines, 2);
                wall[0].color = sf::Color::Black;
                wall[0].position = sf::Vector2f(x*cellWidth, y*cellHeight);
                wall[1].color = sf::Color::Black;
                wall[1].position = sf::Vector2f(x*cellWidth, (y+1)*cellHeight);
                target.draw(wall);
            }
        }
    };

std::vector<Spot> r(row);
std::vector<std::vector<Spot>> plane(col, r);
std::vector<Spot*> available;
std::vector<Spot*> unavailable;
std::vector<Spot*> cellStack;

Spot* start = &plane[0][0];
Spot* finish = &plane[col-1][row-1];

void checkAvailablity(Spot* cell) {
    available.push_back(cell);
    cell -> setColor(sf::Color(0, 183, 204));
}

void checkUnavailablity(Spot* cell) {
    unavailable.push_back(cell);
    cell -> setColor(sf::Color::Red);
}

bool checkEachWall(Spot* h, Spot* vert) {
    int forw = h -> x - vert -> x;
    int back = h -> y - vert -> y;
    if (forw==1) {
        return h -> walls[3];
    }
    if (forw==-1) {
        return h -> walls[1];
    }
    if (back==1) {
        return h -> walls[0];
    }
    return h -> walls[2];
}

void removeEachWall(Spot* h, Spot* vert) {
    int forw = h -> x - vert -> x;
    int back = h -> y - vert -> y;
    if (forw==1) {
        h -> walls[3] = false;
        vert -> walls[1] = false;
    }
    if (forw==-1) {
        h -> walls[1] = false;
        vert -> walls[3] = false;
    }
    if (back==1) {
        h -> walls[0] = false;
        vert -> walls[2] = false;
    }
    if (back==-1) {
        h -> walls[2] = false;
        vert -> walls[0] = false;
    }
}

bool checkIfVisited(Spot* cell, std::vector<Spot*> &vec) {
    for (int i=0; i<(int)vec.size(); ++i) {
        if (vec[i]==cell)
            return true;
    }
    return false;
}

void removeVisited(Spot* cell, std::vector<Spot*> &vec) {
    for (int i=0; i<(int)vec.size(); ++i) {
        if (vec[i]==cell)
            vec.erase(vec.begin()+i);
    }
}

float dist(Spot* h, Spot* vert) {
    return std::sqrt((h -> x - vert -> x)*(h -> x - vert -> x) + 
                    (h -> y - vert -> y)*(h -> y - vert -> y));    
}

void input() {
    // Press "G" to generate maze
    bool keyGeneratePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::G);
    // Press "Enter" to visualize
    bool keyVisualizePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
    if (!generated && keyGeneratePressed) {
        if (state==GRID)
                state = GENERATE;
    }
    if (!visualized && keyVisualizePressed) {
        if (state==CALCULATE) {
            checkAvailablity(start);
            state = VISITING;
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Maze Generation Algorithm");
    std::srand(std::time(nullptr));
    for (int i=0; i<col; ++i) {
        for (int j=0; j<row; ++j)
            plane[i][j].coordinates(i, j);
    }
    
    Spot* curr = &plane[0][0];
    Spot* next = nullptr;
    curr -> visited = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type==sf::Event::Closed)
                window.close();
        }
        window.clear();
        input();

        if (state==GENERATE) {
            curr -> addAdjoin(plane);
            if (curr -> adjoin.size()>0) {
                float rand = static_cast <float> (std::rand())/static_cast <float> (RAND_MAX);
                int x = std::floor(rand*curr -> adjoin.size());
                next = curr -> adjoin[x];
                curr -> adjoin.erase(curr -> adjoin.begin()+x);
            }
            if (next!=nullptr) {
                next -> visited = true;
                cellStack.push_back(curr);
                
                removeEachWall(curr, next);
                curr = next;
                next = nullptr;
            }
            else {
                if (cellStack.size()==0) {
                    state = CALCULATE;
                    for (int i=0; i<col; ++i) {
                        for (int j=0; j<row; ++j)
                            plane[i][j].generateMaze(plane);
                    }
                }
                else {
                    curr = cellStack[cellStack.size()-1];
                    cellStack.pop_back();
                }
            }
        }
        if (state==VISITING) {
            if (available.size()>0) {
                int found = 0;
                for (int i=0; i<(int)available.size(); ++i) {
                    if (available[i] -> top<available[found] -> top)
                        found = i;
                }
                curr = available[found];
                if (available[found]==finish) 
                    state = VISITED;
                
                removeVisited(curr, available);
                checkUnavailablity(curr);
                
                for (int i=0; i<(int)curr -> adjoin.size(); ++i) {
                    if (!checkIfVisited(curr -> adjoin[i], unavailable) && !checkEachWall(curr, curr -> adjoin[i])) {
                        float d = curr -> down+1;
                        if (checkIfVisited(curr -> adjoin[i], available)) {
                            if (d<curr -> adjoin[i] -> down) {
                                curr -> adjoin[i] -> down = d;
                                curr -> adjoin[i] -> prev = curr;
                            }
                        }
                        else {
                            curr -> adjoin[i] -> down = d;
                            checkAvailablity(curr -> adjoin[i]);
                            curr -> adjoin[i] -> down = dist(curr -> adjoin[i], finish);
                            curr -> adjoin[i] -> prev = curr;
                        }
                        curr -> adjoin[i] -> top = curr -> adjoin[i] -> down+curr -> adjoin[i] -> dir;
                    }   
                }
            }
            else 
                state = VISITED;
            
            for (int i=0; i<(int)unavailable.size(); ++i) {
                unavailable[i] -> setColor(sf::Color(0, 183, 204));
            }     
        }
        if (state==VISITING || state==VISITED) {
            while (curr -> prev!=0) {
                curr -> setColor(sf::Color(0, 183, 204));
                    curr = curr -> prev;
            }
            curr -> setColor(sf::Color(180,180,255));
        }

        for (int i=0; i<col; ++i) {
            for (int j=0; j<row; ++j) {
                if (state==GENERATE) {
                    if (plane[i][j].visited)
                        plane[i][j].setColor(sf::Color(255,20,147));
                    curr -> setColor(sf::Color::Blue);
                }
                else if (state==CALCULATE) 
                    plane[i][j].setColor(sf::Color(255,20,147));
                
                window.draw(plane[i][j]);
            }
        }
        window.display();
    }
    return 0;
}
