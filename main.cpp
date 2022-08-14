#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using olc::Sprite;
using olc::vf2d;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "The world is falling"; }

 public:
  void ResetGame() {
    vCameraPos = vf2d(0, 0);
    vPlayerPos = vf2d(10, 10);
    vPlayerVel = vf2d(0, 0);
    fFallTimer = -9999;
    fPlatformTimer = 0.1;
    fAnimTimer = 0;
    sLevel = sLevels[levelIndex];
    objects.clear();
    if (levelIndex == 3) {
      objects.push_back(std::make_pair(3, vf2d(ScreenWidth() - magnet.width * 2 - 10, ScreenHeight() / 2 - magnet.height)));
    }
  }

  void MovePlayer(float fElapsedTime) {
    onGround = false;
    vPlayerPos.y += vPlayerVel.y * fElapsedTime;
    if (Collides()) {
      if (vPlayerVel.y > 0) {
        onGround = true;
      }
      while (Collides()) {
        vPlayerPos.y -= vPlayerVel.y > 0 ? 0.1 : -0.1;
      }
      vPlayerVel.y = 0;
    }
    vPlayerPos.x += vPlayerVel.x * fElapsedTime;
    if (Collides()) {
      while (Collides()) {
        vPlayerPos.x -= vPlayerVel.x > 0 ? 0.1 : -0.1;
      }
      if ((GetKey(olc::W).bHeld || GetKey(olc::UP).bHeld || GetKey(olc::SPACE).bHeld) && (GetKey(olc::DOWN).bHeld || GetKey(olc::S).bHeld)) {
        vPlayerVel.y = -fJump;
        bPlayerFlipped = vPlayerVel.x > 0;
        vPlayerVel.x = vPlayerVel.x > 0 ? -fBounceVel : fBounceVel;
      } else {
        vPlayerVel.x = 0;
      }
    }
    if (vPlayerPos.y > getHeight() * 16) {
      ResetGame();
    }
  }

  bool Collides() {
    for (int x = (vPlayerPos.x + 1) / 16; x < (vPlayerPos.x + vPlayerSize.x - 2) / 16; x++) {
      for (int y = vPlayerPos.y + 1; y < vPlayerPos.y + vPlayerSize.y - 2; y++) {
        if (strchr("E_", getTile(x, (y - getPlatform()) / 16)) != nullptr) {
          if (fPlatformTimer > 0) {
            fPlatformTimer = 0;
          }
          vPlayerVel.x = 0.3;
          return true;
        }
        if (strchr("#GM", getTile(x, (y - getFall(x)) / 16)) != nullptr) {
          return true;
        }
      }
    }
    return false;
  }

  void UpdatePlayer(float fElapsedTime) {
    if (GetKey(olc::A).bHeld || GetKey(olc::LEFT).bHeld) {
      vPlayerVel.x -= fPlayerAccel * fElapsedTime;
      bPlayerFlipped = true;
    }
    if (GetKey(olc::D).bHeld || GetKey(olc::RIGHT).bHeld) {
      vPlayerVel.x += fPlayerAccel * fElapsedTime;
      bPlayerFlipped = false;
    }
    if (GetKey(olc::W).bHeld || GetKey(olc::UP).bHeld || GetKey(olc::SPACE).bHeld) {
      if (onGround) {
        vPlayerVel.y = -fJump;
      }
    }
    vPlayerVel.y += fGravity * fElapsedTime;
    vPlayerVel *= 0.996;
    vPlayerVel = vPlayerVel.min(vMaxVel).max(vMinVel);
    MovePlayer(fElapsedTime);
    if (vPlayerVel.x != 0 && fFallTimer < 0) {
      fFallTimer = 0;
    }
    vCameraPos = (vPlayerPos - vf2d(ScreenWidth(), ScreenHeight()) / 2 + vPlayerSize / 2).min(vf2d(getWidth() * 16, getHeight() * 16) - vf2d(ScreenWidth(), ScreenHeight())).max(vf2d(0, 0));
  }

  bool OnUserCreate() override {
    vPlayerSize = vf2d(32, 32);
    textures.LoadFromFile("Assets/Textures.png");
    idle.LoadFromFile("Assets/Idle.png");
    jump.LoadFromFile("Assets/Jump.png");
    run.LoadFromFile("Assets/Run.png");
    sun.LoadFromFile("Assets/Sun.png");
    cloud.LoadFromFile("Assets/Cloud.png");
    lava.LoadFromFile("Assets/Lava.png");
    magnet.LoadFromFile("Assets/Magnet.png");
    spike.LoadFromFile("Assets/Spike.png");
    ResetGame();
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    fAnimTimer += fElapsedTime;
    if (levelIndex == 3 && fAnimTimer < 2 && bFirstTime) {
      draw();
      SetPixelMode(Pixel::ALPHA);
      FillRect(0, 0, ScreenWidth(), ScreenHeight(), Pixel(0, 0, 0, std::max(255 - fAnimTimer * 128, 0.f)));
      SetPixelMode(Pixel::NORMAL);
      std::string text = "Change you speed!";
      DrawString((vf2d(ScreenWidth(), ScreenHeight()) - GetTextSize(text)) / 2, text);
      return true;
    } else {
      bFirstTime = false;
    }
    if (fPlatformTimer > 0) {
      fFallTimer += fElapsedTime;
      fObjectTimer += fElapsedTime;
    } else {
      fPlatformTimer -= fElapsedTime;
      if (fPlatformTimer < -10) {
        if (levelIndex == 2) {
          int pos = (fPlatformTimer + 10) * 2 * -(vPlayerPos.y - vCameraPos.y - magnet.height);
          if (fPlatformTimer < -13) {
            levelIndex++;
            bFirstTime = true;
            ResetGame();
            return true;
          }
          if (fPlatformTimer < -11) {
            Clear(olc::BLACK);
            return true;
          }
          draw();
          drawAnimation(&magnet, vf2d(vPlayerPos.x - vCameraPos.x, pos), vf2d(magnet.width, magnet.height), false);
          return true;
        }
        if (levelIndex == 3) {
          Clear(olc::BLACK);
          std::string text =
              "Congratulations!\n"
              "You made it to the end!\n"
              "Developed by\n"
              "InfiniteCoder";
          int y = 0;
          size_t pos = 0;
          std::string token;
          while ((pos = text.find("\n")) != std::string::npos) {
            token = text.substr(0, pos);
            DrawString(vf2d((ScreenWidth() - GetTextSize(token).x) / 2, y), token);
            y += GetTextSize(token).y + 2;
            text.erase(0, pos + 1);
          }
          DrawString(vf2d((ScreenWidth() - GetTextSize(text).x) / 2, y), text);
          return true;
        }
        std::string text1 = "Congratulations!", text2 = "You made it to the next level!";
        int y = (ScreenHeight() - GetTextSize(text1).y) / 2;
        DrawString(vf2d((ScreenWidth() - GetTextSize(text1).x) / 2, y), text1);
        y += GetTextSize(text1).y + 2;
        DrawString(vf2d((ScreenWidth() - GetTextSize(text2).x) / 2, y), text2);
        if (fPlatformTimer < -12) {
          levelIndex++;
          ResetGame();
        }
        return true;
      }
    }
    UpdatePlayer(fElapsedTime);
    if (fObjectTimer > (levelIndex == 3 ? 5 : levelIndex == 2 ? 8 : 3)) {
      fObjectTimer = 0;
      if (levelIndex == 1) {
        if (rand() % 100 < 60) {
          objects.push_back(std::make_pair(0, vf2d(vPlayerPos.x + rand() % 64 - 32, vCameraPos.y - 32)));
        } else {
          objects.push_back(std::make_pair(1, vf2d(vPlayerPos.x + rand() % 64 - 32, vCameraPos.y - 32)));
        }
      } else if (levelIndex == 2) {
        objects.push_back(std::make_pair(2, vf2d(vPlayerPos.x + rand() % 64 - 32, vCameraPos.y - 32)));
      } else if (levelIndex == 3) {
        objects.push_back(std::make_pair(4, vf2d(vPlayerPos.x + getSpikeOffset(vCameraPos.y - 32), vCameraPos.y - 32)));
      }
    }

    for (auto& object : objects) {
      if (object.first != 3) {
        object.second.y += fObjectGravity * fElapsedTime * (object.first == 4 ? 2 : 1);
      }
      vf2d size = object.first == 0 ? vf2d(49, 12) : object.first == 2 ? vf2d(48, 48) : vf2d(32, 32);
      if (vPlayerPos.x < (object.second + size).x && object.second.x < (vPlayerPos + vPlayerSize).x && (vPlayerPos + vPlayerSize).y > object.second.y && (object.second + size).y > vPlayerPos.y) {
        ResetGame();
      }
    }

    objects.erase(std::remove_if(objects.begin(), objects.end(), [&](const auto& object) { return object.first != 3 && object.second.y > getHeight() * 16; }), objects.end());

    draw();
    return true;
  }

  void draw() {
    Clear(levelColors[levelIndex]);
    for (int y = vCameraPos.y / 16; y < (vCameraPos.y + ScreenHeight()) / 16 + 1; y++) {
      for (int x = vCameraPos.x / 16; x < (vCameraPos.x + ScreenWidth()) / 16 + 1; x++) {
        if (getTile(x, y) == '#') {
          drawPatch(x, y, 0, 0);
        } else if (getTile(x, y) == 'G') {
          drawPatch(x, y, 2, 0);
        } else if (getTile(x, y) == 'M') {
          DrawPartialSprite(x * 16 - vCameraPos.x, y * 16 + getFall(x) - vCameraPos.y, &textures, 64, 0, 16, 16);
        } else if (getTile(x, y) == 'E') {
          DrawPartialSprite(x * 16 - vCameraPos.x, y * 16 + getPlatform() - vCameraPos.y, &textures, 0, 32, 32, 9);
        }
      }
    }

    for (auto& object : objects) {
      vf2d size = object.first == 0 ? vf2d(49, 12) : object.first == 2 ? vf2d(48, 48) : object.first == 3 ? vf2d(magnet.width, magnet.height) : object.first == 4 ? vf2d(7, 7) : vf2d(32, 32);
      if (object.first == 0) {
        drawAnimation(&cloud, object.second - vCameraPos, size);
      } else if (object.first == 1) {
        drawAnimation(&sun, object.second - vCameraPos, size);
      } else if (object.first == 2) {
        drawAnimation(&lava, object.second - vCameraPos, size);
      } else if (object.first == 3) {
        drawAnimation(&magnet, object.second, size, false, 2);
      } else if (object.first == 4) {
        drawAnimation(&spike, object.second - vCameraPos, size, false, 3);
      }
    }

    drawAnimation(vPlayerVel.y < 0 ? &jump : std::abs(vPlayerVel.x) < 1 ? &idle : &run, vPlayerPos - vCameraPos, vPlayerSize, bPlayerFlipped);
  }

  char getTile(int x, int y) {
    if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight()) return ' ';
    return sLevel[y * getWidth() + x];
  }

  int getWidth() { return sLevel.length() / getHeight(); }
  int getHeight() { return 24; }

  int getFall(int x) { return levelIndex == 3 ? 0 : std::max(fFallTimer * 128 - x * 32, 0.f); }
  int getPlatform() { return std::max(fPlatformTimer, -10.f) * 16 + 7; }
  float getSpikeOffset(float y) {
    float t = (y - vPlayerPos.y) / -fObjectGravity / 2;
    return vPlayerVel.x * t;
  }

 private:
  std::vector<std::pair<int, vf2d>> objects;
  Sprite sun, cloud, lava, magnet, spike;
  Sprite textures, idle, jump, run;
  vf2d vPlayerPos;
  vf2d vPlayerVel;
  vf2d vPlayerSize;
  vf2d vCameraPos;
  vf2d vMaxVel = vf2d(500, 350), vMinVel = vf2d(-500, -800);
  float fPlayerAccel = 800, fGravity = 1500, fObjectGravity = 30, fBounceVel = 400, fJump = 800;
  float fFallTimer, fPlatformTimer, fAnimTimer, fObjectTimer;
  bool onGround, bPlayerFlipped, bFirstTime;
  int levelIndex = 0;

  Pixel levelColors[4] = {olc::BLACK, Pixel(0, 170, 228), olc::BLACK, olc::BLACK};
  std::string sLevel;
  std::string sLevels[4] = {
      "                                                                                                           "
      "                                                                                                           "
      "                                                                                                           "
      "                                                                                                           "
      "                                                                                                           "
      "                                                      ##########    ######                                 "
      "                                                      ##########    ######                                 "
      "                                              ####    ##########    ######                                 "
      "                                                 #    ##########    ######                                 "
      "                                                 #    ##########    ######                                 "
      "                   ###                           #    ##########    ######                                 "
      "                                                 #    ##########    ######                                 "
      "                                                 #    ##########    ######                                 "
      "                                                      ##########    ######                                 "
      "                                                      ##########    ######                            E_   "
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################"
      "###########################################################################################################",
      /***********************************************************************************************************/
      "                                                                                                           "
      "                                                                                                           "
      "                                                                                                           "
      "                                                            GGGGGGGGG                                      "
      "                                                            GGGGGGGGG                                      "
      "                                                            GGGGGGGGG                                      "
      "                                                            GGGGGGGGG                                      "
      "                                                            GGGGGGGGG                                      "
      "                                                  GGGG      GGGGGGGGG                                      "
      "                                                     G      GGGGGGGGG                                      "
      "                                   GGGGGGGGGG        G      GGGGGGGGG                                      "
      "                                   GGGGGGGGGG        G      GGGGGGGGG                                      "
      "                         GGG       GGGGGGGGGG        G      GGGGGGGGG                                      "
      "                                   GGGGGGGGGG               GGGGGGGGG                                      "
      "                                   GGGGGGGGGG               GGGGGGGGG                                 E_   "
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
      "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",
      /***********************************************************************************************************/
      "                                                                                                           "
      "                                                                                                           "
      "                                                                                                           "
      "                                           ####         ####                                               "
      "                                           ####         ####                                               "
      "                                           ####         ####                                               "
      "                               ###         ####         ####                                               "
      "                                           ####         ####                                               "
      "                                     ###   ####         ####                                               "
      "                                           ####         ####                                               "
      "                            ###            ####         ####                                               "
      "                                           ####         ####                                               "
      "                                ####       ####         ####                                               "
      "                                           ####         ####                                               "
      "                                           ####         ####                                          E_   "
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM",
      /***********************************************************************************************************/
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                               "
      "                                                                                                                                                                                                                                          E_   "
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"};

  void drawQuater(int x, int y, int tx, int ty, int x1, int y1) {  // Thanks Kofybreak (Youtube) for this idea
    if (getTile(x + x1 * 2 - 1, y) != getTile(x, y) && getTile(x, y + y1 * 2 - 1) != getTile(x, y)) {
      DrawPartialSprite(x * 16 + x1 * 16 / 2 - vCameraPos.x, y * 16 + y1 * 16 / 2 - vCameraPos.y + getFall(x), &textures, tx * 16 + x1 * 8, ty * 16 + y1 * 8, 8, 8);
    } else if (getTile(x + x1 * 2 - 1, y) != getTile(x, y) && getTile(x, y + y1 * 2 - 1) == getTile(x, y)) {
      DrawPartialSprite(x * 16 + x1 * 16 / 2 - vCameraPos.x, y * 16 + y1 * 16 / 2 - vCameraPos.y + getFall(x), &textures, tx * 16 + x1 * 8, ty * 16 + 16 + y1 * 8, 8, 8);
    } else if (getTile(x + x1 * 2 - 1, y) == getTile(x, y) && getTile(x, y + y1 * 2 - 1) != getTile(x, y)) {
      DrawPartialSprite(x * 16 + x1 * 16 / 2 - vCameraPos.x, y * 16 + y1 * 16 / 2 - vCameraPos.y + getFall(x), &textures, tx * 16 + 16 + x1 * 8, ty * 16 + 16 + y1 * 8, 8, 8);
    } else if (getTile(x + x1 * 2 - 1, y) == getTile(x, y) && getTile(x, y + y1 * 2 - 1) == getTile(x, y)) {
      DrawPartialSprite(x * 16 + x1 * 16 / 2 - vCameraPos.x, y * 16 + y1 * 16 / 2 - vCameraPos.y + getFall(x), &textures, tx * 16 + 16 + x1 * 8, ty * 16 + y1 * 8, 8, 8);
    }
  }

  void drawPatch(int x, int y, int tx, int ty) {
    drawQuater(x, y, tx, ty, 0, 0);
    drawQuater(x, y, tx, ty, 1, 0);
    drawQuater(x, y, tx, ty, 0, 1);
    drawQuater(x, y, tx, ty, 1, 1);
  }

  void drawAnimation(Sprite* sprite, vf2d pos, vf2d size, bool flip = false, int scale = 1) {
    SetPixelMode(Pixel::MASK);
    DrawPartialSprite(pos, sprite, vf2d(int(fAnimTimer * 10) % int(sprite->width / size.x) * size.x, 0), size, scale, flip ? Sprite::Flip::HORIZ : Sprite::Flip::NONE);
    SetPixelMode(Pixel::NORMAL);
  }
};

int main() {
  Game game;
  if (game.Construct(256, 256, 2, 2)) game.Start();

  return 0;
}
