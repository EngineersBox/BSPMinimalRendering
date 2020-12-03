#pragma once

#include <string>
#include <functional>

#include "../../rendering/buffering/PBO.cpp"
#include "../../rendering/colour/Colours.cpp"
#include "../../rendering/drawing/RasterText.hpp"
#include "../../rendering/drawing/DrawingUtils.hpp"
#include "../base/IBaseElement.cpp"
#include "../../logic/id/IDGenerator.cpp"
#include "../../rendering/Globals.hpp"

using namespace std;

namespace GUI {

enum BUTTON_STATE : int {
    CLICKED = 0,
    NOT_CLICKED = 1,
    HOVERED = 2
};

typedef function<void(int_id)> ButtonCallback;

class Button : public IBaseElement {
    public:
        Button();
        Button(string text, ButtonCallback callback);
        Button(int x, int y, int width, int height, string text, ButtonCallback callback);
        Button(int x, int y, int width, int height, string text, Colour::RGB background_colour, Colour::RGB hover_colour, Colour::RGB click_colour, ButtonCallback callback);

        void render(Rendering::PBO& pbo);
        void render();

        void setBackgroundColour(Colour::RGB colour);
        void setHoverColour(Colour::RGB colour);
        void setClickColour(Colour::RGB colour);

        Colour::RGB getBackgroundColour();
        Colour::RGB getHoverColour();
        Colour::RGB getClickColour();

        void setX(int x);
        void setY(int y);
        void setWidth(int width);
        void setHeight(int height);
        void setText(string text);

        int getX();
        int getY();
        int getWidth();
        int getHeight();
        string getText();
        void handleMouse(int button, int state, int posx, int posy);
    private:
        bool inside(int posx, int ypos);

        string text;
        BUTTON_STATE state = BUTTON_STATE::NOT_CLICKED;
        Colour::RGB hover_colour;
        Colour::RGB click_colour;
        ButtonCallback callback;
};

Button::Button() : Button(0, 0, 0, 0, "Button", [](int_id id) {}){};

Button::Button(string text, ButtonCallback callback):
	Button(0, 0, 0, 0, text, callback)
{};

Button::Button(int x, int y, int width, int height, string text,ButtonCallback callback):
	Button(x, y, width, height, text, Colour::RGB_Grey, Colour::RGB_White, Colour::RGB_Black, callback)
{};

Button::Button(int x, int y, int width, int height, string text,
			   Colour::RGB background_colour,
			   Colour::RGB hover_colour, Colour::RGB click_colour,
			   ButtonCallback callback):
	IBaseElement(x, y, width, height, background_colour)
{
	this->text = text;
	this->hover_colour = hover_colour;
	this->click_colour = click_colour;
	this->callback = callback;
};

void Button::setBackgroundColour(Colour::RGB colour) {
  	this->background_colour = colour;
};

void Button::setHoverColour(Colour::RGB colour) {
  	this->hover_colour = colour;
};

void Button::setClickColour(Colour::RGB colour) {
  	this->click_colour = colour;
};

void Button::setX(int x) { this->x = x; };

void Button::setY(int y) { this->y = y; };

void Button::setWidth(int width) { this->width = width; };

void Button::setHeight(int height) { this->height = height; };

void Button::setText(string text) { this->text = text; };

Colour::RGB Button::getBackgroundColour() {
  	return this->background_colour;
};

Colour::RGB Button::getHoverColour() { return this->hover_colour; };

Colour::RGB Button::getClickColour() { return this->click_colour; };

int Button::getX() { return this->x; };

int Button::getY() { return this->y; };

int Button::getWidth() { return this->width; };

int Button::getHeight() { return this->height; };

string Button::getText() { return this->text; };

void Button::render(Rendering::PBO &pbo) {
	Colour::RGB render_colour = this->background_colour;
	switch (this->state) {
		case BUTTON_STATE::CLICKED:
			render_colour = this->click_colour;
			break;
		case BUTTON_STATE::HOVERED:
			render_colour = this->hover_colour;
			break;
		case BUTTON_STATE::NOT_CLICKED:
			render_colour = this->background_colour;
			break;
		default:
			render_colour = this->background_colour;
	}
	for (int x_pos = this->x; x_pos < this->x + this->width; x_pos++) {
		for (int y_pos = this->y; y_pos < this->y + this->height; y_pos++) {
			pbo.pushToBuffer(x_pos, y_pos, render_colour);
		}
	}
	displayText(
		this->x + IDIV_2(this->width - glutBitmapLength(GLUT_BITMAP_HELVETICA_12, reinterpret_cast<const unsigned char *>(this->text.c_str()))),
		this->y + IDIV_2(this->height + 6), Colour::RGB_Red,
		GLUT_BITMAP_HELVETICA_12, this->text);
};

void Button::render() {
	Colour::RGB render_colour = this->background_colour;
	switch (this->state) {
		case BUTTON_STATE::CLICKED:
			render_colour = this->click_colour;
			break;
  		case BUTTON_STATE::HOVERED:
			render_colour = this->hover_colour;
			break;
  		case BUTTON_STATE::NOT_CLICKED:
			render_colour = this->background_colour;
			break;
  		default:
			render_colour = this->background_colour;
  	}
	GLint current_colour[4];
	glGetIntegerv(GL_CURRENT_COLOR, current_colour);
	render_colour.toColour4d();
	drawRectangle(this->x, this->y, this->width, this->height);
	glColor3i(current_colour[0], current_colour[1], current_colour[2]);
	displayText(
		this->x + IDIV_2(this->width - glutBitmapLength(GLUT_BITMAP_HELVETICA_12, reinterpret_cast<const unsigned char *>(this->text.c_str()))),
		this->y + IDIV_2(this->height + 6), Colour::RGB_Red,
		GLUT_BITMAP_HELVETICA_12, this->text);
}

bool Button::inside(int posx, int posy) {
  	return (this->x <= posx && posx <= this->x + this->width && this->y <= posy &&
		  	posy <= this->y + this->height);
}

void Button::handleMouse(int button, int state, int posx, int posy) {
	if (inside(posx, posy)) {
		if (state == GLUT_DOWN &&(button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON || button == GLUT_MIDDLE_BUTTON)) {
			this->state = BUTTON_STATE::CLICKED;
			this->callback(this->id);
		} else {
			this->state = BUTTON_STATE::HOVERED;
		}
	} else {
		this->state = BUTTON_STATE::NOT_CLICKED;
	}
}
}