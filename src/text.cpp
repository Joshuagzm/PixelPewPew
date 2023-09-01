#include "include/text.h"
#include "include/functs.h"

//constructor
gameText::gameText(const char * message, float size)
{
    textValue = message;
    textSize = size;
    dimensions = { MeasureTextEx(GetFontDefault(),textValue, textSize, textMeasureFactor)};
}

//returns true if text has been clicked on
bool gameText::isClickedOn(Vector2 mousePos)
{
    return isHoveredOver(mousePos)
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

//returns true if mouse is hovered over text
bool gameText::isHoveredOver(Vector2 mousePos)
{
    return isMouseInRect(mousePos, position.x, position.y, dimensions);
}

void gameText::drawToScreen()
{
    DrawText(TextFormat(textValue), position.x, position.y, textSize, textColor);
}

//sets the x position of the text to be centered between two points
void gameText::setCenterX(int xLeft, int xRight)
{
    position.x = xLeft + (xRight - xLeft)/2 - dimensions.x/2;
}

//sets the y position of the text to be centered between two points
void gameText::setCenterY(int yTop, int yBottom)
{
    position.y = yTop + (yTop - yBottom)/2 - dimensions.y/2;
}

//changes color on hover from baseTextColor to altTextColor
void gameText::colorOnHover(Vector2 mousePos)
{
    if(isHoveredOver(mousePos)){
        textColor = altTextColor;
    }else{
        textColor = baseTextColor;
    }
}

//sets the x position of the text to be centered between two points
void gameText::setCenterX()
{
    position.x = screenWidth/2 - dimensions.x/2;
}

//sets the y position of the text to be centered between two points
void gameText::setCenterY()
{
    position.y = screenHeight/2 - dimensions.y/2;
}