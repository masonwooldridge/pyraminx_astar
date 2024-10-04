#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
#include <random>

using namespace std;

// Variables for isClockwise
bool clockwise = true;
bool counterClockwise = false;

// Enum for representing colors
enum Color { RED, GREEN, YELLOW, BLUE };
enum FaceNum {FRONT = 0, LEFT = 1, RIGHT = 2, BOTTOM = 3};

// Function to convert colors to ANSI escape codes
string getColorCode(Color color) {
    switch (color) {
        case RED: return "\033[91m"; // Red text
        case GREEN: return "\033[92m"; // Green text
        case YELLOW: return "\033[93m"; // Yellow text
        case BLUE: return "\033[94m"; // Blue text
        default: return "\033[0m"; // Default text color
    }
}

// Function to reset text color
string resetColor() {
    return "\033[0m";
}

// Class representing a single triangle
class Triangle {
public:
    Color color;
    std::stack<int> positionHistory;

    Triangle(Color initialColor) : color(initialColor) {}

    void setColor(Color newColor) {
        color = newColor;
    }

    void pushPosition(int position) {
        positionHistory.push(position);
    }

    void popPosition() {
        if (!positionHistory.empty()) {
            positionHistory.pop();
        }
    }

    int getCurrentPosition() const {
        if (!positionHistory.empty()) {
            return positionHistory.top();
        }
        return -1;
    }
};

// Define the output operator for Triangle
ostream& operator<<(ostream& os, const Triangle& triangle) {
    os << getColorCode(triangle.color);
    switch (triangle.color) {
        case RED: os << 'R'; break;
        case GREEN: os << 'G'; break;
        case YELLOW: os << 'Y'; break;
        case BLUE: os << 'B'; break;
        default: os << '?'; break;
    }
    os << resetColor();
    return os;
}

// Class representing a single face of the Pyraminx
class Face {
public:
    vector<vector<Triangle> > face;
    Color faceColor;

    Face(Color initialColor, FaceNum faceNum) {
        face.resize(4); // 4 rows
        for (int i = 0; i < 4; ++i) {
            face[i].resize(2 * i + 1, Triangle(initialColor));
            for (int j = 0; j < face[i].size(); ++j) {
                int position = -1;
                if (i == 0) {
                    position = faceNum * 16;
                } else if (i == 1) {
                    position = (faceNum * 16) + (j + 1);
                } else if (i == 2) {
                    position = (faceNum * 16) + (j + 4);
                } else {
                    position = (faceNum * 16) + (j + 9);
                }
                face[i][j].pushPosition(position);
            }
        }
    }

    //Get row for at the index for specified face
    vector<Triangle>& getRow(int rowIndex) {
        return face[rowIndex];
    }

    const vector<Triangle>& getRow(int rowIndex) const {
        return face[rowIndex];
    }

    int countColorFace(Color color) {
        //count the number of triangles of one color on a file
        int colorCount = 0;
        for (int i = 0; i < 4; ++i) {
            for (size_t j = 0; j < getRow(i).size(); ++j) {
                if(getRow(i)[j].color == color) {
                    colorCount = colorCount + 1;
                }
            }
        }
        return (colorCount);
    }

    void findFaceColor() {
        //Find the color of the face by determining which color has the most number of triangles on the face
        int redNum = countColorFace(RED);
        int blueNum = countColorFace(BLUE);
        int greenNum = countColorFace(GREEN);
        int yellowNum = countColorFace(YELLOW);
        if (((redNum >= blueNum)&&(redNum >= greenNum))&&(redNum >= yellowNum)) {
            faceColor = RED;
        }
        else if ((blueNum >= greenNum)&&(blueNum >= yellowNum)) {
            faceColor = BLUE;
        }
        else if (greenNum >= yellowNum) {
            faceColor = GREEN;
        }
        else {
            faceColor = YELLOW;
        }
    }

    int countWrongTriangles() {
        //count the number of triangles on the wrong face
        //for example: the number of blue triangles on  the red face
        int wrongColor = 0;
        for (int i = 0; i < 4; ++i) {
            for (size_t j = 0; j < getRow(i).size(); ++j) {
                if(getRow(i)[j].color != faceColor) {
                    wrongColor = wrongColor + 1;
                }
            }
        }
        return (wrongColor);
    }
};

// Class representing the entire Pyraminx
class Pyraminx {
public:
    Face front, left, right, bottom;

    Pyraminx() 
        : front(RED, FRONT), left(GREEN, LEFT), right(YELLOW, RIGHT), bottom(BLUE, BOTTOM) {}

    //Prints the pyraminx with the folded out view
    void printPyraminx() {
        printTopRows(left.getRow(3), bottom.getRow(0), right.getRow(3), 0);
        printTopRows(left.getRow(2), bottom.getRow(1), right.getRow(2), 1);
        printTopRows(left.getRow(1), bottom.getRow(2), right.getRow(1), 2);
        printTopRows(left.getRow(0), bottom.getRow(3), right.getRow(0), 3);
        printBottomRows(front.getRow(3), 0);
        printBottomRows(front.getRow(2), 1);
        printBottomRows(front.getRow(1), 2);
        printBottomRows(front.getRow(0), 3);
    }
    
    // This has same set up for the other tips
    void rotateTopTip(bool isClockwise) {

    // Get references to the triangles
    Triangle& leftTop = left.getRow(3)[0];
    Triangle& rightTop = right.getRow(3)[6];
    Triangle& frontTop = front.getRow(0)[0];

    // Get current colors
    Color leftTopColor = leftTop.color;
    Color rightTopColor = rightTop.color;
    Color frontTopColor = frontTop.color;

    // Get current positions
    int leftTopPos = leftTop.getCurrentPosition();
    int rightTopPos = rightTop.getCurrentPosition();
    int frontTopPos = frontTop.getCurrentPosition();

    if (isClockwise) {
        // Update positions for clockwise rotation
        leftTop.setColor(frontTopColor);
        leftTop.pushPosition(frontTopPos);

        rightTop.setColor(leftTopColor);
        rightTop.pushPosition(leftTopPos);

        frontTop.setColor(rightTopColor);
        frontTop.pushPosition(rightTopPos);
    } else {
        // Update positions for counter-clockwise rotation
        leftTop.setColor(rightTopColor);
        leftTop.pushPosition(rightTopPos);

        rightTop.setColor(frontTopColor);
        rightTop.pushPosition(frontTopPos);

        frontTop.setColor(leftTopColor);
        frontTop.pushPosition(leftTopPos);
    }
}

    void rotateLeftTip(bool isClockwise) {
    
    Triangle& leftTop = left.getRow(0)[0];
    Triangle& bottomTop = bottom.getRow(3)[0];
    Triangle& frontTop = front.getRow(3)[0];

    
    Color leftTopColor = leftTop.color;
    Color bottomTopColor = bottomTop.color;
    Color frontTopColor = frontTop.color;

    int leftTopPos = leftTop.getCurrentPosition();
    int bottomTopPos = bottomTop.getCurrentPosition();
    int frontTopPos = frontTop.getCurrentPosition();

    if (isClockwise) {
        
        leftTop.setColor(bottomTopColor);
        leftTop.pushPosition(bottomTopPos);

        bottomTop.setColor(frontTopColor);
        bottomTop.pushPosition(frontTopPos);

        frontTop.setColor(leftTopColor);
        frontTop.pushPosition(leftTopPos);

    } else {
        
        leftTop.setColor(frontTopColor);
        leftTop.pushPosition(frontTopPos);

        frontTop.setColor(bottomTopColor);
        frontTop.pushPosition(bottomTopPos);

        bottomTop.setColor(leftTopColor);
        bottomTop.pushPosition(leftTopPos);
    }
}

    void rotateRightTip(bool isClockwise) {

    Triangle& bottomTop = bottom.getRow(3)[6];
    Triangle& rightTop = right.getRow(0)[0];
    Triangle& frontTop = front.getRow(3)[6];

    Color bottomTopColor = bottomTop.color;
    Color rightTopColor = rightTop.color;
    Color frontTopColor = frontTop.color;

    int bottomTopPos = bottomTop.getCurrentPosition();
    int rightTopPos = rightTop.getCurrentPosition();
    int frontTopPos = frontTop.getCurrentPosition();

    if (isClockwise) {

        bottomTop.setColor(rightTopColor);
        bottomTop.pushPosition(rightTopPos);

        rightTop.setColor(frontTopColor);
        rightTop.pushPosition(frontTopPos);

        frontTop.setColor(bottomTopColor);
        frontTop.pushPosition(bottomTopPos);

    } else {

        bottomTop.setColor(frontTopColor);
        bottomTop.pushPosition(frontTopPos);

        frontTop.setColor(rightTopColor);
        frontTop.pushPosition(rightTopPos);

        rightTop.setColor(bottomTopColor);
        rightTop.pushPosition(bottomTopPos);
    }
}

    void rotateBackTip(bool isClockwise) {

    Triangle& bottomTop = bottom.getRow(0)[0];
    Triangle& rightTop = right.getRow(3)[0];
    Triangle& leftTop = left.getRow(3)[6];

    Color bottomTopColor = bottomTop.color;
    Color rightTopColor = rightTop.color;
    Color leftTopColor = leftTop.color;

    int bottomTopPos = bottomTop.getCurrentPosition();
    int rightTopPos = rightTop.getCurrentPosition();
    int leftTopPos = leftTop.getCurrentPosition();

    if (isClockwise) {

        bottomTop.setColor(leftTopColor);
        bottomTop.pushPosition(leftTopPos);

        rightTop.setColor(bottomTopColor);
        rightTop.pushPosition(bottomTopPos);

        leftTop.setColor(rightTopColor);
        leftTop.pushPosition(rightTopPos);

    } else {

        bottomTop.setColor(rightTopColor);
        bottomTop.pushPosition(rightTopPos);

        rightTop.setColor(leftTopColor);
        rightTop.pushPosition(leftTopPos);

        leftTop.setColor(bottomTopColor);
        leftTop.pushPosition(bottomTopPos);
    }
}

    // This has same set up for the other 2nd and 3rd row rotations
    void rotateFrontSecondRow(bool isClockwise) {
        vector<Triangle>& frontRow = front.getRow(1);

        // Create vectors to hold pointers to the triangles
        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        // Populate the vectors with pointers to the appropriate triangles
        leftRow.push_back(&left.getRow(3)[2]);
        leftRow.push_back(&left.getRow(3)[1]);
        leftRow.push_back(&left.getRow(2)[0]);

        rightRow.push_back(&right.getRow(2)[4]);
        rightRow.push_back(&right.getRow(3)[5]);
        rightRow.push_back(&right.getRow(3)[4]);

        // Temp vectors for colors and positions
        vector<Color> tempColors(3);
        vector<int> tempPositions(3);

        // Rotations
        if (isClockwise) {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(rightRow[i]->color);
                frontRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(leftRow[i]->color);
                frontRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateLeftSecondRow(bool isClockwise) {
        vector<Triangle>& leftRow = left.getRow(1);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        frontRow.push_back(&front.getRow(3)[2]);
        frontRow.push_back(&front.getRow(3)[1]);
        frontRow.push_back(&front.getRow(2)[0]);

        bottomRow.push_back(&bottom.getRow(2)[0]);
        bottomRow.push_back(&bottom.getRow(3)[1]);
        bottomRow.push_back(&bottom.getRow(3)[2]);

        vector<Color> tempColors(3);
        vector<int> tempPositions(3);

        if (isClockwise) {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(bottomRow[i]->color);
                leftRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(frontRow[i]->color);
                leftRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateRightSecondRow(bool isClockwise) {
        vector<Triangle>& rightRow = right.getRow(1);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        frontRow.push_back(&front.getRow(3)[5]);
        frontRow.push_back(&front.getRow(3)[4]);
        frontRow.push_back(&front.getRow(2)[4]);

        bottomRow.push_back(&bottom.getRow(2)[4]);
        bottomRow.push_back(&bottom.getRow(3)[5]);
        bottomRow.push_back(&bottom.getRow(3)[4]);

        vector<Color> tempColors(3);
        vector<int> tempPositions(3);

        if (isClockwise) {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(frontRow[i]->color);
                rightRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(bottomRow[i]->color);
                rightRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateBottomSecondRow(bool isClockwise) {
        vector<Triangle>& bottomRow = bottom.getRow(1);

        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        leftRow.push_back(&left.getRow(2)[4]);
        leftRow.push_back(&left.getRow(3)[4]);
        leftRow.push_back(&left.getRow(3)[5]);

        rightRow.push_back(&right.getRow(2)[0]);
        rightRow.push_back(&right.getRow(3)[1]);
        rightRow.push_back(&right.getRow(3)[2]);

        vector<Color> tempColors(3);
        vector<int> tempPositions(3);

        if (isClockwise) {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(leftRow[i]->color);
                bottomRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 3; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(rightRow[i]->color);
                bottomRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }
    
    void rotateFrontThirdRow(bool isClockwise) {
        vector<Triangle>& frontRow = front.getRow(2);

        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        leftRow.push_back(&left.getRow(3)[4]);
        leftRow.push_back(&left.getRow(3)[3]);
        leftRow.push_back(&left.getRow(2)[2]);
        leftRow.push_back(&left.getRow(2)[1]);
        leftRow.push_back(&left.getRow(1)[0]);
        
        rightRow.push_back(&right.getRow(1)[2]);
        rightRow.push_back(&right.getRow(2)[3]);
        rightRow.push_back(&right.getRow(2)[2]);
        rightRow.push_back(&right.getRow(3)[3]);
        rightRow.push_back(&right.getRow(3)[2]);

        vector<Color> tempColors(5);
        vector<int> tempPositions(5);

        if (isClockwise) {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(rightRow[i]->color);
                frontRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(leftRow[i]->color);
                frontRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateLeftThirdRow(bool isClockwise) {
        vector<Triangle>& leftRow = left.getRow(2);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        frontRow.push_back(&front.getRow(3)[4]);
        frontRow.push_back(&front.getRow(3)[3]);
        frontRow.push_back(&front.getRow(2)[2]);
        frontRow.push_back(&front.getRow(2)[1]);
        frontRow.push_back(&front.getRow(1)[0]);

        bottomRow.push_back(&bottom.getRow(3)[4]);
        bottomRow.push_back(&bottom.getRow(3)[3]);
        bottomRow.push_back(&bottom.getRow(2)[2]);
        bottomRow.push_back(&bottom.getRow(2)[1]);
        bottomRow.push_back(&bottom.getRow(1)[0]);

        vector<Color> tempColors(5);
        vector<int> tempPositions(5);

        if (isClockwise) {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(frontRow[i]->color);
                leftRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(bottomRow[i]->color);
                leftRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateRightThirdRow(bool isClockwise) {
        vector<Triangle>& rightRow = right.getRow(2);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        frontRow.push_back(&front.getRow(1)[2]);
        frontRow.push_back(&front.getRow(2)[3]);
        frontRow.push_back(&front.getRow(2)[2]);
        frontRow.push_back(&front.getRow(3)[3]);
        frontRow.push_back(&front.getRow(3)[2]);

        bottomRow.push_back(&bottom.getRow(3)[2]);
        bottomRow.push_back(&bottom.getRow(3)[3]);
        bottomRow.push_back(&bottom.getRow(2)[2]);
        bottomRow.push_back(&bottom.getRow(2)[3]);
        bottomRow.push_back(&bottom.getRow(1)[2]);

        vector<Color> tempColors(5);
        vector<int> tempPositions(5);

        if (isClockwise) {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(bottomRow[i]->color);
                rightRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(frontRow[i]->color);
                rightRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }

    void rotateBottomThirdRow(bool isClockwise) {
        vector<Triangle>& bottomRow = bottom.getRow(2);

        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        leftRow.push_back(&left.getRow(3)[2]);
        leftRow.push_back(&left.getRow(3)[3]);
        leftRow.push_back(&left.getRow(2)[2]);
        leftRow.push_back(&left.getRow(2)[3]);
        leftRow.push_back(&left.getRow(1)[2]);

        rightRow.push_back(&right.getRow(1)[0]);
        rightRow.push_back(&right.getRow(2)[1]);
        rightRow.push_back(&right.getRow(2)[2]);
        rightRow.push_back(&right.getRow(3)[3]);
        rightRow.push_back(&right.getRow(3)[4]);

        vector<Color> tempColors(5);
        vector<int> tempPositions(5);

        if (isClockwise) {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(rightRow[i]->color);
                bottomRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);
            }
        } else {
            for (int i = 0; i < 5; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(leftRow[i]->color);
                bottomRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
        }
    }
    
    // This has same set up for the other 4th row rotations
    void rotateFrontFourthRow(bool isClockwise) {
        vector<Triangle>& frontRow = front.getRow(3);

        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        // Vector for entire face that changes
        vector<Triangle*> bottomFace;

        leftRow.push_back(&left.getRow(3)[6]);
        leftRow.push_back(&left.getRow(3)[5]);
        leftRow.push_back(&left.getRow(2)[4]);
        leftRow.push_back(&left.getRow(2)[3]);
        leftRow.push_back(&left.getRow(1)[2]);
        leftRow.push_back(&left.getRow(1)[1]);
        leftRow.push_back(&left.getRow(0)[0]);

        rightRow.push_back(&right.getRow(0)[0]);
        rightRow.push_back(&right.getRow(1)[1]);
        rightRow.push_back(&right.getRow(1)[0]);
        rightRow.push_back(&right.getRow(2)[1]);
        rightRow.push_back(&right.getRow(2)[0]);
        rightRow.push_back(&right.getRow(3)[1]);
        rightRow.push_back(&right.getRow(3)[0]);

        // Push into face vector in order
        bottomFace.push_back(&bottom.getRow(0)[0]);
        bottomFace.push_back(&bottom.getRow(1)[0]);
        bottomFace.push_back(&bottom.getRow(1)[1]);
        bottomFace.push_back(&bottom.getRow(1)[2]);
        bottomFace.push_back(&bottom.getRow(2)[0]);
        bottomFace.push_back(&bottom.getRow(2)[1]);
        bottomFace.push_back(&bottom.getRow(2)[2]);
        bottomFace.push_back(&bottom.getRow(2)[3]);
        bottomFace.push_back(&bottom.getRow(2)[4]);
        bottomFace.push_back(&bottom.getRow(3)[0]);
        bottomFace.push_back(&bottom.getRow(3)[1]);
        bottomFace.push_back(&bottom.getRow(3)[2]);
        bottomFace.push_back(&bottom.getRow(3)[3]);
        bottomFace.push_back(&bottom.getRow(3)[4]);
        bottomFace.push_back(&bottom.getRow(3)[5]);
        bottomFace.push_back(&bottom.getRow(3)[6]);
        
        // Temp for face colors and positions
        vector<Color> tempBottomColors;
        vector<int> tempBottomPositions;

        // New positions for rotated face
        int changedPositionsClockwise[] = {9,11,10,4,13,12,6,5,1,15,14,8,7,3,2,0};
        int changedPositionsCounterClockwise[] = {15,8,14,13,3,7,6,12,11,0,2,1,5,4,10,9};

        // Store in temp
        for (std::vector<Triangle*>::iterator it = bottomFace.begin(); it != bottomFace.end(); ++it) {
            Triangle* triangle = *it;
            if (triangle != nullptr) {
                tempBottomColors.push_back(triangle->color);
                tempBottomPositions.push_back(triangle->getCurrentPosition());
            }
        }

        vector<Color> tempColors(7);
        vector<int> tempPositions(7);

        if (isClockwise) {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(rightRow[i]->color);
                frontRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);

            }

            // Change face positions and colors to rotated values
            for (int i = 0; i < 16; i++) {
                bottomFace[i]->setColor(tempBottomColors[changedPositionsClockwise[i]]);
                bottomFace[i]->pushPosition(tempBottomPositions[changedPositionsClockwise[i]]);
            }

        } else {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = frontRow[i].color;
                tempPositions[i] = frontRow[i].getCurrentPosition();

                frontRow[i].setColor(leftRow[i]->color);
                frontRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
            for (int i = 0; i < 16; i++) {
                bottomFace[i]->setColor(tempBottomColors[changedPositionsCounterClockwise[i]]);
                bottomFace[i]->pushPosition(tempBottomPositions[changedPositionsCounterClockwise[i]]);
            }
        }
    }
    
    void rotateLeftFourthRow(bool isClockwise) {
        vector<Triangle>& leftRow = left.getRow(3);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        vector<Triangle*> rightFace;

        frontRow.push_back(&front.getRow(3)[6]);
        frontRow.push_back(&front.getRow(3)[5]);
        frontRow.push_back(&front.getRow(2)[4]);
        frontRow.push_back(&front.getRow(2)[3]);
        frontRow.push_back(&front.getRow(1)[2]);
        frontRow.push_back(&front.getRow(1)[1]);
        frontRow.push_back(&front.getRow(0)[0]);

        bottomRow.push_back(&bottom.getRow(0)[0]);
        bottomRow.push_back(&bottom.getRow(1)[1]);
        bottomRow.push_back(&bottom.getRow(1)[2]);
        bottomRow.push_back(&bottom.getRow(2)[3]);
        bottomRow.push_back(&bottom.getRow(2)[4]);
        bottomRow.push_back(&bottom.getRow(3)[5]);
        bottomRow.push_back(&bottom.getRow(3)[6]);

        rightFace.push_back(&right.getRow(0)[0]);
        rightFace.push_back(&right.getRow(1)[0]);
        rightFace.push_back(&right.getRow(1)[1]);
        rightFace.push_back(&right.getRow(1)[2]);
        rightFace.push_back(&right.getRow(2)[0]);
        rightFace.push_back(&right.getRow(2)[1]);
        rightFace.push_back(&right.getRow(2)[2]);
        rightFace.push_back(&right.getRow(2)[3]);
        rightFace.push_back(&right.getRow(2)[4]);
        rightFace.push_back(&right.getRow(3)[0]);
        rightFace.push_back(&right.getRow(3)[1]);
        rightFace.push_back(&right.getRow(3)[2]);
        rightFace.push_back(&right.getRow(3)[3]);
        rightFace.push_back(&right.getRow(3)[4]);
        rightFace.push_back(&right.getRow(3)[5]);
        rightFace.push_back(&right.getRow(3)[6]);

        vector<Color> tempRightColors;
        vector<int> tempRightPositions;
        int changedPositionsCounterClockwise[] = {9,11,10,4,13,12,6,5,1,15,14,8,7,3,2,0};
        int changedPositionsClockwise[] = {15,8,14,13,3,7,6,12,11,0,2,1,5,4,10,9};

        for (std::vector<Triangle*>::iterator it = rightFace.begin(); it != rightFace.end(); ++it) {
            Triangle* triangle = *it;
            if (triangle != nullptr) {
                tempRightColors.push_back(triangle->color);
                tempRightPositions.push_back(triangle->getCurrentPosition());
            }
        }

        vector<Color> tempColors(7);
        vector<int> tempPositions(7);

        if (isClockwise) {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(bottomRow[i]->color);
                leftRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);

            }
            for (int i = 0; i < 16; i++) {
                rightFace[i]->setColor(tempRightColors[changedPositionsClockwise[i]]);
                rightFace[i]->pushPosition(tempRightPositions[changedPositionsClockwise[i]]);
            }

        } else {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = leftRow[i].color;
                tempPositions[i] = leftRow[i].getCurrentPosition();

                leftRow[i].setColor(frontRow[i]->color);
                leftRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);
            }
            for (int i = 0; i < 16; i++) {
                rightFace[i]->setColor(tempRightColors[changedPositionsCounterClockwise[i]]);
                rightFace[i]->pushPosition(tempRightPositions[changedPositionsCounterClockwise[i]]);
            }
        }
    }

    void rotateRightFourthRow(bool isClockwise) {
        vector<Triangle>& rightRow = right.getRow(3);

        vector<Triangle*> frontRow;
        vector<Triangle*> bottomRow;

        vector<Triangle*> leftFace;

        frontRow.push_back(&front.getRow(0)[0]);
        frontRow.push_back(&front.getRow(1)[1]);
        frontRow.push_back(&front.getRow(1)[0]);
        frontRow.push_back(&front.getRow(2)[1]);
        frontRow.push_back(&front.getRow(2)[0]);
        frontRow.push_back(&front.getRow(3)[1]);
        frontRow.push_back(&front.getRow(3)[0]);

        bottomRow.push_back(&bottom.getRow(3)[0]);
        bottomRow.push_back(&bottom.getRow(3)[1]);
        bottomRow.push_back(&bottom.getRow(2)[0]);
        bottomRow.push_back(&bottom.getRow(2)[1]);
        bottomRow.push_back(&bottom.getRow(1)[0]);
        bottomRow.push_back(&bottom.getRow(1)[1]);
        bottomRow.push_back(&bottom.getRow(0)[0]);

        leftFace.push_back(&left.getRow(0)[0]);
        leftFace.push_back(&left.getRow(1)[0]);
        leftFace.push_back(&left.getRow(1)[1]);
        leftFace.push_back(&left.getRow(1)[2]);
        leftFace.push_back(&left.getRow(2)[0]);
        leftFace.push_back(&left.getRow(2)[1]);
        leftFace.push_back(&left.getRow(2)[2]);
        leftFace.push_back(&left.getRow(2)[3]);
        leftFace.push_back(&left.getRow(2)[4]);
        leftFace.push_back(&left.getRow(3)[0]);
        leftFace.push_back(&left.getRow(3)[1]);
        leftFace.push_back(&left.getRow(3)[2]);
        leftFace.push_back(&left.getRow(3)[3]);
        leftFace.push_back(&left.getRow(3)[4]);
        leftFace.push_back(&left.getRow(3)[5]);
        leftFace.push_back(&left.getRow(3)[6]);

        vector<Color> tempLeftColors;
        vector<int> tempLeftPositions;
        int changedPositionsCounterClockwise[] = {9,11,10,4,13,12,6,5,1,15,14,8,7,3,2,0};
        int changedPositionsClockwise[] = {15,8,14,13,3,7,6,12,11,0,2,1,5,4,10,9};

        for (std::vector<Triangle*>::iterator it = leftFace.begin(); it != leftFace.end(); ++it) {
            Triangle* triangle = *it;
            if (triangle != nullptr) {
                tempLeftColors.push_back(triangle->color);
                tempLeftPositions.push_back(triangle->getCurrentPosition());
            }
        }

        vector<Color> tempColors(7);
        vector<int> tempPositions(7);

        if (isClockwise) {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(frontRow[i]->color);
                rightRow[i].pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(bottomRow[i]->color);
                frontRow[i]->pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(tempColors[i]);
                bottomRow[i]->pushPosition(tempPositions[i]);

            }
            for (int i = 0; i < 16; i++) {
                leftFace[i]->setColor(tempLeftColors[changedPositionsClockwise[i]]);
                leftFace[i]->pushPosition(tempLeftPositions[changedPositionsClockwise[i]]);
            }

        } else {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = rightRow[i].color;
                tempPositions[i] = rightRow[i].getCurrentPosition();

                rightRow[i].setColor(bottomRow[i]->color);
                rightRow[i].pushPosition(bottomRow[i]->getCurrentPosition());

                bottomRow[i]->setColor(frontRow[i]->color);
                bottomRow[i]->pushPosition(frontRow[i]->getCurrentPosition());

                frontRow[i]->setColor(tempColors[i]);
                frontRow[i]->pushPosition(tempPositions[i]);
            }
            for (int i = 0; i < 16; i++) {
                leftFace[i]->setColor(tempLeftColors[changedPositionsCounterClockwise[i]]);
                leftFace[i]->pushPosition(tempLeftPositions[changedPositionsCounterClockwise[i]]);
            }
        }
    }

    void rotateBottomFourthRow(bool isClockwise) {
        vector<Triangle>& bottomRow = bottom.getRow(3);

        // Create vectors to hold pointers to the triangles
        vector<Triangle*> leftRow;
        vector<Triangle*> rightRow;

        vector<Triangle*> frontFace;

        // Populate the vectors with pointers to the appropriate triangles
        leftRow.push_back(&left.getRow(3)[0]);
        leftRow.push_back(&left.getRow(3)[1]);
        leftRow.push_back(&left.getRow(2)[0]);
        leftRow.push_back(&left.getRow(2)[1]);
        leftRow.push_back(&left.getRow(1)[0]);
        leftRow.push_back(&left.getRow(1)[1]);
        leftRow.push_back(&left.getRow(0)[0]);

        rightRow.push_back(&right.getRow(0)[0]);
        rightRow.push_back(&right.getRow(1)[1]);
        rightRow.push_back(&right.getRow(1)[2]);
        rightRow.push_back(&right.getRow(2)[3]);
        rightRow.push_back(&right.getRow(2)[4]);
        rightRow.push_back(&right.getRow(3)[5]);
        rightRow.push_back(&right.getRow(3)[6]);

        frontFace.push_back(&front.getRow(0)[0]);
        frontFace.push_back(&front.getRow(1)[0]);
        frontFace.push_back(&front.getRow(1)[1]);
        frontFace.push_back(&front.getRow(1)[2]);
        frontFace.push_back(&front.getRow(2)[0]);
        frontFace.push_back(&front.getRow(2)[1]);
        frontFace.push_back(&front.getRow(2)[2]);
        frontFace.push_back(&front.getRow(2)[3]);
        frontFace.push_back(&front.getRow(2)[4]);
        frontFace.push_back(&front.getRow(3)[0]);
        frontFace.push_back(&front.getRow(3)[1]);
        frontFace.push_back(&front.getRow(3)[2]);
        frontFace.push_back(&front.getRow(3)[3]);
        frontFace.push_back(&front.getRow(3)[4]);
        frontFace.push_back(&front.getRow(3)[5]);
        frontFace.push_back(&front.getRow(3)[6]);

        vector<Color> tempFrontColors;
        vector<int> tempFrontPositions;
        int changedPositionsClockwise[] = {9,11,10,4,13,12,6,5,1,15,14,8,7,3,2,0};
        int changedPositionsCounterClockwise[] = {15,8,14,13,3,7,6,12,11,0,2,1,5,4,10,9};

        for (std::vector<Triangle*>::iterator it = frontFace.begin(); it != frontFace.end(); ++it) {
            Triangle* triangle = *it;
            if (triangle != nullptr) {
                tempFrontColors.push_back(triangle->color);
                tempFrontPositions.push_back(triangle->getCurrentPosition());
            }
        }

        vector<Color> tempColors(7);
        vector<int> tempPositions(7);

        if (isClockwise) {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(rightRow[i]->color);
                bottomRow[i].pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(leftRow[i]->color);
                rightRow[i]->pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(tempColors[i]);
                leftRow[i]->pushPosition(tempPositions[i]);

            }
            for (int i = 0; i < 16; i++) {
                frontFace[i]->setColor(tempFrontColors[changedPositionsClockwise[i]]);
                frontFace[i]->pushPosition(tempFrontPositions[changedPositionsClockwise[i]]);
            }

        } else {
            for (int i = 0; i < 7; i++) {
                tempColors[i] = bottomRow[i].color;
                tempPositions[i] = bottomRow[i].getCurrentPosition();

                bottomRow[i].setColor(leftRow[i]->color);
                bottomRow[i].pushPosition(leftRow[i]->getCurrentPosition());

                leftRow[i]->setColor(rightRow[i]->color);
                leftRow[i]->pushPosition(rightRow[i]->getCurrentPosition());

                rightRow[i]->setColor(tempColors[i]);
                rightRow[i]->pushPosition(tempPositions[i]);
            }
            for (int i = 0; i < 16; i++) {
                frontFace[i]->setColor(tempFrontColors[changedPositionsCounterClockwise[i]]);
                frontFace[i]->pushPosition(tempFrontPositions[changedPositionsCounterClockwise[i]]);
            }
        }
    
    }

    // Prints the top 4 rows (left, bottom, and right faces)
    void printTopRows(const vector<Triangle>& leftRow, const vector<Triangle>& bottomRow, const vector<Triangle>& rightRow, int rowIndex) const {
        for (int i = 0; i < rowIndex * 2; ++i) {
            cout << " ";
        }
        for (size_t i = 0; i < leftRow.size(); ++i) {
            cout << leftRow[i] << " ";
        }
        for (size_t i = 0; i < bottomRow.size(); ++i) {
            cout << bottomRow[i] << " ";
        }
        for (size_t i = 0; i < rightRow.size(); ++i) {
            if (i == rightRow.size() - 1) {
                cout << rightRow[i];
            } else {
                cout << rightRow[i] << " ";
            }
        }
        for (int i = 0; i < rowIndex * 2; ++i) {
            cout << " ";
        }
        cout << endl;
    }

    // Prints the bottom 4 rows (front face)
    void printBottomRows(const vector<Triangle>& frontRow, int rowIndex) const {
        for (int i = 0; i < rowIndex * 2 + 8; ++i) {
            cout << " ";
        }
        for (size_t i = 0; i < frontRow.size(); ++i) {
            if (i == frontRow.size() - 1) {
                cout << frontRow[i];
            } else {
                cout << frontRow[i] << " ";
            }
        }
        for (int i = 0; i < rowIndex * 2 + 8; ++i) {
            cout << " ";
        }
        cout << endl;
    }

    int findHeuristic() {
        //calculate the heuristic for the pyraminx
        double wrongTriangles = 0;
        int heuristic = 0;
        //Find which color each face has the most of
        front.findFaceColor();
        left.findFaceColor();
        right.findFaceColor();
        bottom.findFaceColor();
        //Calculates how many triangles are in the wrong position
        wrongTriangles = front.countWrongTriangles() + left.countWrongTriangles() + right.countWrongTriangles() + bottom.countWrongTriangles();
        //Heuristic function calls the ceiling function
        heuristic = ceil(wrongTriangles/21);
        return(heuristic);
    }

    void randomizer() {
        //randomizer to perform random moves 0-31
        int random = 0;
        random_device rd;
        uniform_int_distribution<int> dist(0,31);
        random = dist(rd);
        cout << "Random: " << random << endl;
        switch(random) {
            case 0:
                rotateTopTip(clockwise);
                break;
            case 1:
                rotateTopTip(counterClockwise);
                break;
            case 2:
                rotateLeftTip(clockwise);
                break;
            case 3:
                rotateLeftTip(counterClockwise);
                break;
            case 4:
                rotateRightTip(clockwise);
                break;
            case 5:
                rotateRightTip(counterClockwise);
                break;
            case 6:
                rotateBackTip(clockwise);
                break;
            case 7:
                rotateBackTip(counterClockwise);
                break;
            case 8:
                rotateFrontSecondRow(clockwise);
                break;
            case 9:
                rotateFrontSecondRow(counterClockwise);
                break;
            case 10:
                rotateLeftSecondRow(clockwise);
                break;
            case 11:
                rotateLeftSecondRow(counterClockwise);
                break;
            case 12:
                rotateRightSecondRow(clockwise);
                break;
            case 13:
                rotateRightSecondRow(counterClockwise);
                break;
            case 14:
                rotateBottomSecondRow(clockwise);
                break;
            case 15:
                rotateBottomSecondRow(counterClockwise);
                break;
            case 16:
                rotateFrontThirdRow(clockwise);
                break;
            case 17:
                rotateFrontThirdRow(counterClockwise);
                break;
            case 18:
                rotateLeftThirdRow(clockwise);
                break;
            case 19:
                rotateLeftThirdRow(counterClockwise);
                break;
            case 20:
                rotateRightThirdRow(clockwise);
                break;
            case 21:
                rotateRightThirdRow(counterClockwise);
                break;
            case 22:
                rotateBottomThirdRow(clockwise);
                break;
            case 23:
                rotateBottomThirdRow(counterClockwise);
                break;
            case 24:
                rotateFrontFourthRow(clockwise);
                break;
            case 25:
                rotateFrontFourthRow(counterClockwise);
                break;
            case 26:
                rotateLeftFourthRow(clockwise);
                break;
            case 27:
                rotateLeftFourthRow(counterClockwise);
                break;
            case 28:
                rotateRightFourthRow(clockwise);
                break;
            case 29:
                rotateRightFourthRow(counterClockwise);
                break;
            case 30:
                rotateBottomFourthRow(clockwise);
                break;
            case 31:
                rotateBottomFourthRow(counterClockwise);
                break;
        }
    }

};

int main() {

    //Handles user input to determine how many random moves to perform
    int userInput = 0;
    cout << "Input the number of random rotations to perform:" << endl;
    cin >> userInput;
    if (userInput < 0) {
        cout << "Invalid input" << endl;
        return 1;
    }

    //creates the pyraminx in the solved state
    Pyraminx pyraminx;
    pyraminx.printPyraminx();
    cout << "Heuristic: " << pyraminx.findHeuristic() << endl;

    //calls the randomizer and prints the random moves
    for (int i = 0; i < userInput; i++) {
        pyraminx.randomizer();
    }
    pyraminx.printPyraminx();
    cout << "Heuristic: " << pyraminx.findHeuristic() << endl;

    return 0;
}
