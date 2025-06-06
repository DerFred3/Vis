#include <GLApp.h>
#include <FontRenderer.h>

class MyGLApp : public GLApp {
public:
  Image image{640,480};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  std::string text;

  MyGLApp() : GLApp{800,800,1,"Color Picker"} {}
  
  Vec3 convertPosFromHSVToRGB(float x, float y) {
    // TODO:
    // enter code here that interprets the mouse's
    // x, y position as H ans S (I suggest to set
    // V to 1.0) and converts that tripple to RGB
    const float h = x * 360.0f;
    const float s = y;
    const float v = 1.0f;
    const float c = v * s;

    const float h_prime = h / 60.0f;
    const float a = c * (1 - std::abs((std::fmod(h_prime, 2.0f)) - 1));

    Vec3 rgb_out;
    if (h_prime < 1) {
        rgb_out = Vec3{c, a, 0.0f};
    } else if (h_prime < 2) {
        rgb_out = Vec3{a, c, 0.0f};
    } else if (h_prime < 3) {
        rgb_out = Vec3{0.0f, c, a};
    } else if (h_prime < 4) {
        rgb_out = Vec3{0.0f, a, c};
    } else if (h_prime < 5) {
        rgb_out = Vec3{a, 0.0f, c};
    } else if (h_prime < 6) {
        rgb_out = Vec3{c, 0.0f, a};
    } else {
        rgb_out = Vec3{0.0f, 0.0f, 0.0f};
    }

    const Vec3 m_vec = Vec3{v-c, v-c, v-c};
    return rgb_out + m_vec;
  }
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb = convertPosFromHSVToRGB(float(x)/image.width, float(y)/image.height);
        image.setNormalizedValue(x,y,0,rgb.r); image.setNormalizedValue(x,y,1,rgb.g);
        image.setNormalizedValue(x,y,2,rgb.b); image.setValue(x,y,3,255);
      }
    }
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glBlendEquation(GL_FUNC_ADD));
    GL(glEnable(GL_BLEND));
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    const Vec3 hsv{float(360*xPosition/s.width),float(1.0-yPosition/s.height),1.0f};
    const Vec3 rgb = convertPosFromHSVToRGB(float(xPosition/s.width), float(1.0-yPosition/s.height));
    std::stringstream ss; ss << "HSV: " << hsv << "  RGB: " << rgb; text = ss.str();
  }
    
  virtual void draw() override {
    drawImage(image);

    const Dimensions dim{ glEnv.getFramebufferSize() };
    fe->render(text, dim.aspect(), 0.03f, {0,-0.9f}, Alignment::Center, {0,0,0,1});
  }
};

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    MyGLApp myApp;
    myApp.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
