#ifndef TEXT_H
#define TEXT_H

#include <raylib.h>

//header file for text related functions and classes

//class to represent a piece of text for display on a raylib screen
class gameText
{
    public:
        const char * textValue  {"Network Config"};
        float textSize {30};
        Vector2 dimensions{0,0};
        Vector2 position{0,0};
        Color textColor{WHITE};
        Color baseTextColor{WHITE};
        Color altTextColor{GREEN};
        const double textMeasureFactor{3.0};

        //functions
        gameText(const char * message, float size);
        bool isClickedOn(Vector2 mousePos);
        bool isHoveredOver(Vector2 mousePos);
        void colorOnHover(Vector2 mousePos);
        void drawToScreen();
        void setCenterX(int xLeft, int xRight);
        void setCenterY(int yTop, int yBottom);
        void setCenterX();
        void setCenterY();
};

#endif