#define GL_SILENCE_DEPRECATION

#include "Raytracer.hpp"

#define SPRITE_U_DIV 1
#define SPRITE_V_DIV 1
#define SPRITE_V_MOVE 0.0

#define DARK_SHADER 8355711

using namespace std;

// Screen
int screenW = 1024;
int screenH = 512;

ResourceManager::TextureLoader texLoader;
HashTable<Texture> textures;
AStar astar;
vector<Coords> *path = new vector<Coords>();
// Player
Player player;

// Map
int mapScreenW = screenW;
int mapScreenH = screenH;

float mapScalingX;
float mapScalingY;

// Configs
ResourceManager::ConfigInit cfgInit;

GameMap gameMap = GameMap();

vector<int> spriteOrder;
vector<double> spriteDistance;

double newTime = 0;
double oldTime = 0;

double frameTime = 0;

Rendering::PBO pixel_buffer_obj;
Rendering::RayBuffer rays;
Rendering::ZBuffer zBuf;
GLuint texid;

///
/// Render the player
///
/// @return void
///
inline static void renderPlayerPos(int sw = screenW, int sh = screenH) {
    int xOffset = minimapCfg.isLeft() ? 0 : sw - (gameMap.map_width * minimapCfg.size);
    int yOffset = minimapCfg.isTop() ? 0 : sh - (gameMap.map_height * minimapCfg.size);

    Colour::RGB_Red.toColour4d();
    glPointSize(8);

    // Draw player point
    glBegin(GL_POINTS);
    glVertex2d(xOffset + (player.x * mapScalingX), yOffset + (player.y * mapScalingY));

    glEnd();

    // Draw direction vector
    renderRay(
        xOffset + (player.x * mapScalingX),
        yOffset + (player.y * mapScalingY),
        xOffset + ((player.x + player.dx * 5) * mapScalingX),
        yOffset + ((player.y + player.dy * 5) * mapScalingY),
        3, Colour::RGB_Red);
}

///
/// Render the map as squares
///
/// @return void
///
static void renderMap2D(int sw = screenW, int sh = screenH) {
    int xOffset = minimapCfg.isLeft() ? 0 : sw - (gameMap.map_width * minimapCfg.size);
    int yOffset = minimapCfg.isTop() ? 0 : sh - (gameMap.map_height * minimapCfg.size);
    int x, y;
    for (y = 0; y < gameMap.map_height; y++) {
        for (x = 0; x < gameMap.map_width; x++) {
            // Change to colour coresponding to map location
            gameMap.getAt(x, y).wf_left.colour.toColour4d();
            drawRectangle(xOffset + x * minimapCfg.size, yOffset + y * minimapCfg.size, minimapCfg.size, minimapCfg.size);
        }
    }
}

inline void drawPixel(int x, int y, Colour::ColorRGB colour) {
    int idx = y * screenW + x;
    pixel_buffer_obj[3 * idx + 0] = colour.r;
    pixel_buffer_obj[3 * idx + 1] = colour.g;
    pixel_buffer_obj[3 * idx + 2] = colour.b;
}

inline static void renderFloorCeiling() {
    float rayDirX0, rayDirY0, rayDirX1, rayDirY1, posZ, rowDistance, floorStepX, floorStepY, floorX, floorY;
    int p, cellX, cellY, tx, ty, checkerBoardPattern;
    Texture tex;
    string floorTexture, ceilingTexture;
    uint32_t color;
    for (int y = screenH - 1; y >= IDIV_2(screenH) + 1; --y) {
        rayDirX0 = player.dx - player.camera.clip_plane_x;
        rayDirY0 = player.dy - player.camera.clip_plane_y;
        rayDirX1 = player.dx + player.camera.clip_plane_x;
        rayDirY1 = player.dy + player.camera.clip_plane_y;

        p = y - IDIV_2(screenH);

        posZ = 0.5 * screenH;

        rowDistance = posZ / p;

        floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenW;
        floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenW;

        floorX = player.x + rowDistance * rayDirX0;
        floorY = player.y + rowDistance * rayDirY0;

        for (int x = screenW - 1; x >= 0; --x) {
            cellX = (int)(floorX);
            cellY = (int)(floorY);

            tx = (int)(renderCfg.texture_width * (floorX - cellX)) & (renderCfg.texture_width - 1);
            ty = (int)(renderCfg.texture_height * (floorY - cellY)) & (renderCfg.texture_height - 1);

            floorX += floorStepX;
            floorY += floorStepY;

            textures.get(gameMap.floor_texture, tex);
            color = tex.texture[renderCfg.texture_width * ty + tx];
            color = (color >> 1) & DARK_SHADER;
            drawPixel(x, screenH - y, Colour::INTtoRGB(color));

            textures.get(gameMap.ceiling_texture, tex);
            color = tex.texture[renderCfg.texture_width * ty + tx];
            color = (color >> 1) & DARK_SHADER;
            drawPixel(x, y - 1, Colour::INTtoRGB(color));
        }
    }
}

inline static void renderWalls() {
    double rayDirX, rayDirY, sideDistX, sideDistY, deltaDistX, deltaDistY, perpWallDist, wallX, step, texPos;
    int mapX, mapY, stepX, stepY, hit, side, lineHeight, drawStart, drawEnd, texX, texY;
    string wallTex;
    uint32_t color;
    Texture tex;
    for (int x = screenW - 1; x >= 0; x--) {
        player.camera.x = 2 * x / double(screenW) - 1;
        rayDirX = player.dx + player.camera.clip_plane_x * player.camera.x;
        rayDirY = player.dy + player.camera.clip_plane_y * player.camera.x;

        mapX = (int)player.x;
        mapY = (int)player.y;

        deltaDistX = abs(1 / rayDirX);
        deltaDistY = abs(1 / rayDirY);

        hit = 0;
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
        }

        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            hit = gameMap.getAt(mapX, mapY).wf_left.texture != "";
        }

        if (side == 0) {
            perpWallDist = (mapX - player.x + IDIV_2((1 - stepX))) / rayDirX;
        } else {
            perpWallDist = (mapY - player.y + IDIV_2((1 - stepY))) / rayDirY;
        }
        lineHeight = (int)(screenH / perpWallDist);

        drawStart = IDIV_2(-lineHeight) + IDIV_2(screenH);
        if (drawStart < 0) {
            drawStart = 0;
        }
        drawEnd = IDIV_2(lineHeight) + IDIV_2(screenH);
        if (drawEnd >= screenH) {
            drawEnd = screenH - 1;
        }
        wallTex = gameMap.getAt(mapX, mapY).wf_left.texture;

        wallX = side == 0 ? player.y + perpWallDist* rayDirY : player.x + perpWallDist * rayDirX;
        wallX -= floor((wallX));

        texX = (int)(wallX * double(renderCfg.texture_width));
        if (side == 0 && rayDirX > 0) {
            texX = renderCfg.texture_width - texX - 1;
        }
        if (side == 1 && rayDirY < 0) {
            texX = renderCfg.texture_width - texX - 1;
        }

        step = 1.0 * renderCfg.texture_height / lineHeight;
        texPos = (drawStart - IDIV_2(screenH) + IDIV_2(lineHeight)) * step;
        for (int y = drawStart; y < drawEnd; y++) {
            texY = (int)texPos & (renderCfg.texture_height - 1);
            texPos += step;
            textures.get(wallTex, tex);
            color = tex.texture[renderCfg.texture_height * texY + texX];
            if (side == 1) {
                color = (color >> 1) & DARK_SHADER;
            }
            drawPixel(x, screenH - y, Colour::INTtoRGB(color));
        }

        zBuf[x] = perpWallDist;
    }
}

inline double sqDist(double ax, double ay, double bx, double by) {
    return pow(bx - ax, 2) + pow(by - ay, 2);
}

inline void sortSprites() {
    int amount = gameMap.sprites.size();
    vector<pair<double, int>> sprites(amount);
    for (int i = amount - 1; i >= 0; i--) {
        sprites[i].first = sqDist(gameMap.sprites[i].location.x, player.x, gameMap.sprites[i].location.y, player.y);
        sprites[i].second = i;
    }
    sort(sprites.begin(), sprites.end());
    for (int i = amount - 1; i >= 0; i--) {
        spriteDistance[i] = sprites[amount - i - 1].first;
        spriteOrder[i] = sprites[amount - i - 1].second;
    }
}

inline static void renderSprites() {
    sortSprites();

    double spriteX, spriteY, transformX, transformY;
    int spriteScreenX, V_MOVEScreen, spriteHeight, drawStartY, drawEndY, spriteWidth, drawStartX, drawEndX, texX, texY, d;
    Texture tex;
    uint32_t color;
    double invDet = 1.0 / (player.camera.clip_plane_x * player.dy - player.dx * player.camera.clip_plane_y);
    for (int i = gameMap.sprites.size() - 1; i >= 0; i--) {
        spriteX = gameMap.sprites[spriteOrder[i]].location.x - player.x;
        spriteY = gameMap.sprites[spriteOrder[i]].location.y - player.y;

        transformX = invDet * (player.dy * spriteX - player.dx * spriteY);
        transformY = invDet * (-player.camera.clip_plane_y * spriteX + player.camera.clip_plane_x * spriteY);

        spriteScreenX = (int)(IDIV_2(screenW) * (1 + transformX / transformY));

        V_MOVEScreen = (int)(SPRITE_V_MOVE / transformY);

        spriteHeight = abs((int)(screenH / (transformY))) / SPRITE_V_DIV;
        drawStartY = -IDIV_2(spriteHeight) + IDIV_2(screenH) + V_MOVEScreen;
        if (drawStartY < 0) {
            drawStartY = 0;
        }
        drawEndY = IDIV_2(spriteHeight) + IDIV_2(screenH) + V_MOVEScreen;
        if (drawEndY >= screenH) {
            drawEndY = screenH - 1;
        }

        spriteWidth = abs((int)(screenH / (transformY))) / SPRITE_U_DIV;
        drawStartX = IDIV_2(-spriteWidth) + spriteScreenX;
        if (drawStartX < 0) {
            drawStartX = 0;
        }
        drawEndX = IDIV_2(spriteWidth) + spriteScreenX;
        if (drawEndX >= screenW) {
            drawEndX = screenW - 1;
        }
        textures.get(gameMap.sprites[spriteOrder[i]].texture, tex);
        for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
            texX = (int)IDIV_256((IMUL_256((stripe - (IDIV_2(-spriteWidth) + spriteScreenX))) * renderCfg.texture_width / spriteWidth));
            if (!(transformY > 0 && stripe > 0 && stripe < screenW && transformY < zBuf[stripe])) {
                continue;
            }
            for (int y = drawStartY; y < drawEndY; y++) {
                d = IMUL_256((y - V_MOVEScreen)) - IMUL_128(screenH) + IMUL_128(spriteHeight);
                texY = IDIV_256((d * renderCfg.texture_height) / spriteHeight);
                color = tex.texture[renderCfg.texture_width * texY + texX];
                if ((color & 0x00FFFFFF) != 0) {
                    drawPixel(stripe, screenH - y, Colour::INTtoRGB(color));
                }
            }
        }
    }
}

inline static void updateTimeTick() {
    oldTime = newTime;
    newTime = glutGet(GLUT_ELAPSED_TIME);
    frameTime = (newTime - oldTime) / 1000.0;
    displayText(10, 10, Colour::RGB_Yellow, to_string(1.0 / frameTime).c_str());
    // IMPL: USE https://learnopengl.com/In-Practice/Text-Rendering TO PRINT OUT THE TEXT TO SCREEN

    player.moveSpeed = frameTime * playerCfg.move_speed;
    player.rotSpeed = frameTime * playerCfg.rotation_speed;
}

static void display(void) {
    if (renderCfg.headless_mode) {
        return;
    }

    if (renderCfg.render_floor_ceiling) {
        renderFloorCeiling();
    }
    if (renderCfg.render_walls) {
        renderWalls();
    }
    if (renderCfg.render_sprites) {
        renderSprites();
    }

    glEnable(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT);

    glTexSubImage2D(GL_TEXTURE_2D,
        0, 0, 0, screenW,screenH,
        GL_RGB, GL_UNSIGNED_BYTE,
        pixel_buffer_obj.data());

    glBegin(GL_QUADS);
        glTexCoord2d(1,1); glVertex2d(0, 0);
        glTexCoord2d(0,1); glVertex2d(screenW, 0);
        glTexCoord2d(0,0); glVertex2d(screenW, screenH);
        glTexCoord2d(1,0); glVertex2d(0, screenH);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

static void __KEY_HANDLER(unsigned char key, int x, int y) {
    if (key == 'w') {
        if (gameMap.getAt((int)(player.x + player.dx * player.moveSpeed), (int)player.y).wf_left.texture == "")
            player.x += player.dx * player.moveSpeed;
        if (gameMap.getAt((int)player.x, (int)(player.y + player.dy * player.moveSpeed)).wf_left.texture == "")
            player.y += player.dy * player.moveSpeed;
    } else if (key == 's') {
        if (gameMap.getAt((int)(player.x - player.dx * player.moveSpeed), (int)player.y).wf_left.texture == "")
            player.x -= player.dx * player.moveSpeed;
        if (gameMap.getAt((int)player.x, (int)(player.y - player.dy * player.moveSpeed)).wf_left.texture == "")
            player.y -= player.dy * player.moveSpeed;
    } else if (key == 'a') {
        double oldDirX = player.dx;
        player.dx = player.dx * cos(-player.rotSpeed) - player.dy * sin(-player.rotSpeed);
        player.dy = oldDirX * sin(-player.rotSpeed) + player.dy * cos(-player.rotSpeed);

        double oldPlaneX = player.camera.clip_plane_x;
        player.camera.clip_plane_x = player.camera.clip_plane_x * cos(-player.rotSpeed) - player.camera.clip_plane_y * sin(-player.rotSpeed);
        player.camera.clip_plane_y = oldPlaneX * sin(-player.rotSpeed) + player.camera.clip_plane_y * cos(-player.rotSpeed);
    } else if (key == 'd') {
        double oldDirX = player.dx;
        player.dx = player.dx * cos(player.rotSpeed) - player.dy * sin(player.rotSpeed);
        player.dy = oldDirX * sin(player.rotSpeed) + player.dy * cos(player.rotSpeed);

        double oldPlaneX = player.camera.clip_plane_x;
        player.camera.clip_plane_x = player.camera.clip_plane_x * cos(player.rotSpeed) - player.camera.clip_plane_y * sin(player.rotSpeed);
        player.camera.clip_plane_y = oldPlaneX * sin(player.rotSpeed) + player.camera.clip_plane_y * cos(player.rotSpeed);
    }
    glutPostRedisplay();
}

///
/// Initialise the display rendering and player position
///
/// @return void
///
void __INIT() {
    cfgInit.initAll(playerCfg, minimapCfg, loggingCfg, renderCfg);
    if (minimapCfg.enable) {
        mapScreenW = IDIV_2(mapScreenW);
    }
    debugContext = GLDebugContext(&loggingCfg);
    debugContext.logAppInfo("Loaded debug context");

    texLoader = ResourceManager::TextureLoader();
    texLoader.loadTextures(textures);
    debugContext.logAppInfo(string("Loaded " + to_string(textures.size()) + " textures"));

    gameMap.readMapFromJSON(MAPS_DIR + "map2.json");

    spriteOrder.resize(gameMap.sprites.size());
    spriteDistance.resize(gameMap.sprites.size());

    rays = Rendering::RayBuffer(playerCfg.fov);
    zBuf = Rendering::ZBuffer(screenW);

    gameMap.wall_width = mapScreenW / gameMap.map_width;
    gameMap.wall_height = mapScreenH / gameMap.map_height;

    mapScalingX = (minimapCfg.size / (float)mapScreenW) * gameMap.map_width;
    mapScalingY = (minimapCfg.size / (float)mapScreenH) * gameMap.map_height;

    astar = AStar(gameMap);
    path = astar.find(gameMap.start, gameMap.end);

    player.moveSpeed = frameTime * playerCfg.move_speed;
    player.rotSpeed = frameTime * playerCfg.rotation_speed;

    // toClearColour(bg_colour);
    gluOrtho2D(0, screenW, screenH, 0);
    player = Player(
        gameMap.start.x,
        gameMap.start.y,
        -1.0,
        0.0,
        0);

    debugContext.logAppInfo("Initialised player object");

    pixel_buffer_obj = Rendering::PBO(screenW * screenH * 3);
    debugContext.logApiInfo("Initialised new PBO of size " + to_string(screenW * screenH * 4) + " [" + to_string(screenW) + "*" + to_string(screenH) + "*3" + "]");

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    debugContext.logApiInfo("Bound PBO texture to id: " + to_string(texid));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenW, screenH, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer_obj.data());
    debugContext.logApiInfo("Allocated PBO texture to from:" + ADDR_OF(*pixel_buffer_obj.data()));
}

static void __WINDOW_RESHAPE(int width, int height) {
    screenW = width;
    screenH = height;
    zBuf.resize(width);
    pixel_buffer_obj.resize(width * height * 3);
}

static void __GLUT_IDLE(void) {
    glutPostRedisplay();
}

///
/// Main execution
///
/// @param int argc: Call value
/// @param char argv: Program parameters
///
/// @return int
///
int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenW, screenH);
    glutCreateWindow("Ray Caster");

    __INIT();
    debugContext.logAppInfo("---- COMPLETED APPLICATION INIT PHASE ----");

    glutDisplayFunc(display);
    debugContext.logApiInfo("Initialised glutDisplayFunc [display] at: " + ADDR_OF(display));
    glutReshapeFunc(__WINDOW_RESHAPE);
    debugContext.logApiInfo("Initialised glutReshapeFunc [__WINDOW_RESHAPE] at: " + ADDR_OF(__WINDOW_RESHAPE));
    glutKeyboardFunc(__KEY_HANDLER);
    debugContext.logApiInfo("Initialised glutKeyboardFunc [__KEY_HANDLER] at: " + ADDR_OF(__KEY_HANDLER));
    glutIdleFunc(__GLUT_IDLE);
    debugContext.logApiInfo("Initialised glutIdleFunc [__GLUT_IDLE] at: " + ADDR_OF(__GLUT_IDLE));
    glutPostRedisplay();
    debugContext.logApiInfo("---- COMPLETED OpenGL/GLUT INIT PHASE ----");
    debugContext.logApiInfo("Started glutMainLoop()");
    glutPostRedisplay();
    glutMainLoop();
    return 0;
}