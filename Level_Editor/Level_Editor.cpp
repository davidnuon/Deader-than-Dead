#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include "src/Button.h"
#include "src/StringInput.h"
#include "src/Sprites.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;
SDL_Event event;

TTF_Font* font = NULL;
SDL_Color textColor = {0,0,0};

//Number of buttons and the button array declaration.
const int BUTTONSIZE = 4;
Button buttonArray[BUTTONSIZE];

//Offset for the menu.
const int yMenuOffset = 18;
//Vector of the sprites used.
std::vector<Sprites> spriteVec;
//Vector to hold the offset of the panning and scrolling.
SDL_Rect posOffset;

SDL_Surface* testSurface = NULL;
std::string currFile = "";
std::string currType = "";
std::string currBackground = "";

SDL_Surface* load_image(std::string filename){
	SDL_Surface* loadedImage = NULL;
	SDL_Surface* optimizedImage = NULL;
	loadedImage = IMG_Load(filename.c_str());
	if(loadedImage != NULL){
		optimizedImage = SDL_DisplayFormatAlpha(loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	return optimizedImage;
}

bool init(){
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return false;
	
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	
	if(screen == NULL)
		return false;

	if(TTF_Init() == -1)
		return false;

	SDL_WM_SetCaption("Deader-than-Dead Level Editor Test", NULL);

	return true;
}

bool load_files(){
	font = TTF_OpenFont("res/arial.ttf", 18);
	if(font == NULL)
		return false;
	return true;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface *destination, SDL_Rect *clip = NULL){
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;
	SDL_BlitSurface(source, clip, destination, &offset);
}

//Apply a surface, but with the offsets built in.
void apply_surface_wOffset(int x, int y, SDL_Surface* source, SDL_Surface *destination, SDL_Rect *clip = NULL){
	SDL_Rect offset;
	offset.x = x + posOffset.x;
	offset.y = y + posOffset.y;
	SDL_BlitSurface(source, clip, destination, &offset);
}

void cleanup(){
	//Frees the sprites.
	for(int i = 0; i < spriteVec.size(); i++){
		SDL_FreeSurface(spriteVec[i].spriteSurface);
	}
	SDL_FreeSurface(background);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}

//Initializes the buttons.
bool initButtons(){
	buttonArray[0].message = TTF_RenderText_Solid(font, "Save", textColor);
	buttonArray[0].box.x = 25;
	buttonArray[0].box.y = 0;
	buttonArray[0].box.w = buttonArray[0].message->w;
	buttonArray[0].box.h = buttonArray[0].message->h;

	buttonArray[1].message = TTF_RenderText_Solid(font, "Load Level", textColor);
	buttonArray[1].box.x = 100 + buttonArray[0].box.x + buttonArray[0].box.w;
	buttonArray[1].box.y = 0;
	buttonArray[1].box.w = buttonArray[1].message->w;
	buttonArray[1].box.h = buttonArray[1].message->h;

	buttonArray[2].message = TTF_RenderText_Solid(font, "Objects", textColor);
	buttonArray[2].box.x = 100 + buttonArray[1].box.x + buttonArray[1].box.w;
	buttonArray[2].box.y = 0;
	buttonArray[2].box.w = buttonArray[2].message->w;
	buttonArray[2].box.h = buttonArray[2].message->h;

	buttonArray[3].message = TTF_RenderText_Solid(font, "Enemies", textColor);
	buttonArray[3].box.x = 100 + buttonArray[2].box.x + buttonArray[2].box.w;
	buttonArray[3].box.y = 0;
	buttonArray[3].box.w = buttonArray[3].message->w;
	buttonArray[3].box.h = buttonArray[3].message->h;

	for(int i = 0; i < BUTTONSIZE; i++){
		if(buttonArray[i].message == NULL){
			return false;
		}
	}

	return true;
}

//Writes the position of the sprites to the file.
bool writeToFile(std::string filename, std::vector<Sprites> vec){
	bool bl = false;
	std::string finalName = "levels/";
	finalName += filename;
	std::ofstream file;
	file.open(finalName.c_str());
	if(file.is_open()){
		bl = true;

		if(background != NULL){
			file << currBackground << '\n';
		}
		for(int i = 0; i < spriteVec.size(); i++){
			file << spriteVec[i].box.x << " ";
			file << spriteVec[i].box.y << " ";
			file << spriteVec[i].box.w << " ";
			file << spriteVec[i].box.h << " ";
			file << spriteVec[i].type << " ";
			file << spriteVec[i].fromFile << '\n';
		}

		file.close();
	}
	return bl;
}

//Gets string input.
std::string gettingStringInput(bool &quit){
	StringInput strIn;
	bool end = false;
	int inputType = 0;
	while(!end && !quit){
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				quit = true;
			}
			if(event.type == SDL_KEYDOWN){
				int inputType = strIn.get_input(event);
				if(inputType == 1){
					SDL_FreeSurface(strIn.textInput);
					strIn.textInput = TTF_RenderText_Solid(font, strIn.getStr().c_str(), textColor);
					std::cout << "Hey" << std::endl;
				}
				if(inputType == 2){
					end = true;
				}
			}
		}

		SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

		strIn.display(SCREEN_WIDTH, SCREEN_HEIGHT, yMenuOffset, screen);
		std::cout << strIn.getStr() << std::endl;

		if(SDL_Flip(screen) == -1){
			std::cout << "hey there" << std::endl;
		}
	}

	return strIn.getStr();
}

//Used to handle panning the screen.
void handle_pan(){
	int x = event.button.x;
	int y = event.button.y;

	//Checks if the mouse is on the window.
	if(x > 0 && x < SCREEN_WIDTH && y > 0 && y < SCREEN_HEIGHT){

		if(background != NULL){
			//Checks to pan right.
			if(x > SCREEN_WIDTH*.8){
				posOffset.x -= 1;
				if(posOffset.x < (background->w * -1) + SCREEN_WIDTH){
					posOffset.x = (background->w * -1) + SCREEN_WIDTH;
				}
			}
			//Checks to pan left.
			else if(x < SCREEN_WIDTH*.2){
				posOffset.x += 1;
				if(posOffset.x > 0){
					posOffset.x = 0;
				}
			}

			//Checks to pan down.
			if(y > SCREEN_HEIGHT*.8){
				posOffset.y -= 1;
				if(posOffset.y < (background->h * -1) + SCREEN_HEIGHT){
					posOffset.y = (background->h * -1) + SCREEN_HEIGHT;
				}
			}
			//Checks to pan up.
			else if(y < SCREEN_HEIGHT*.2){
				posOffset.y += 1;
				if(posOffset.y > 0){
					posOffset.y = 0;
				}
			}
		}
	}
}

int main(int argc, char* argv[]){
	bool quit = false;

	if(init() == false)
		return 1;
	if(load_files() == false)
		return 1;
	if(initButtons() == false)
		return 1;

	//Sets up the location of the menu bar.
	SDL_Rect menuBar;
	menuBar.x = 0;
	menuBar.y = 0;
	menuBar.w = SCREEN_WIDTH;
	menuBar.h = buttonArray[0].message->h;

	//Fills the menuBar rect with white.
	SDL_FillRect(screen, &menuBar, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
	
	while(!quit){
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				quit = true;
			}
			if(event.type == SDL_MOUSEBUTTONUP){
				if(event.button.button == SDL_BUTTON_LEFT){
					int x = event.button.x;
					int y = event.button.y;
					
					//The save function writes the contents of the vector of Sprites to the specified file.
					if(buttonArray[0].check_click(x, y)){
						std::string stringIn = gettingStringInput(quit);
						if(stringIn != "")
							writeToFile(stringIn, spriteVec);
					}
					

					//Checks for a button click.
					if(buttonArray[1].check_click(x, y)){
						std::string stringIn = gettingStringInput(quit);
						stringIn = "res/" + stringIn;
						background = load_image(stringIn);
						if(background != NULL){
							currBackground = stringIn;
							currType = "Background";
						}
					}

					if(buttonArray[2].check_click(x, y)){
						std::string stringIn = gettingStringInput(quit);
						stringIn = "res/" + stringIn;
						testSurface = load_image(stringIn);
						if(testSurface != NULL){
							currFile = stringIn;
							currType = "Object";
						}
					}

					if(buttonArray[3].check_click(x, y)){
						std::string stringIn = gettingStringInput(quit);
						stringIn = "res/" + stringIn;
						testSurface = load_image(stringIn);
						if(testSurface != NULL){
							currFile = stringIn;
							currType = "Enemy";
						}
					}

					//Creates a new sprite object if the click was not on the menubar.
					if(y > yMenuOffset && currFile != ""){
						Sprites temp;
						temp.spriteSurface = load_image(currFile);
						temp.box.w = temp.spriteSurface->w;
						temp.box.h = temp.spriteSurface->h;
						temp.box.x = (x + (posOffset.x * -1))/16 * 16;
						temp.box.y = (y + (posOffset.y * -1))/16 * 16;

						temp.clip.x = 0;
						temp.clip.y = 0;
						temp.clip.w = 32;
						temp.clip.h = 32;

						temp.type = currType;
						temp.fromFile = currFile;
						
						spriteVec.push_back(temp);
					}
				}
			}
		}
		
		//Used to pan the screen.
		handle_pan();

		//Wipes the screen.
		SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

		//Displays the background if one was loaded.
		if(background != NULL){
			apply_surface_wOffset(0, yMenuOffset, background, screen);
		}

		//Displays the sprites to the screen.
		for(int i = 0; i < spriteVec.size(); i++){
			apply_surface_wOffset(spriteVec[i].box.x, spriteVec[i].box.y, spriteVec[i].spriteSurface, screen, &spriteVec[i].clip);
		}

		//Displays a white bar across the top of the screen for the menu bar.
		SDL_FillRect(screen, &menuBar, SDL_MapRGB(screen->format, 0x3F, 0x00, 0x1F));

		//Displays the buttons ontop of the menu bar.
		for(int i = 0; i < BUTTONSIZE; i++){
			SDL_Rect buttonBackground;
			buttonBackground.x = buttonArray[i].box.x - 5;
			buttonBackground.y = buttonArray[i].box.y;
			buttonBackground.w = buttonArray[i].box.w + 10;
			buttonBackground.h = buttonArray[i].box.h + 1;
			SDL_FillRect(screen, &buttonBackground, SDL_MapRGB(screen->format, 0xFF, 0x3F, 0x5F));
			apply_surface(buttonArray[i].box.x, buttonArray[i].box.y, buttonArray[i].message, screen);
		}

		if(SDL_Flip(screen) == -1)
			return 1;
	}

	cleanup();
	
	return 0;
}