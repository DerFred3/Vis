#include <iomanip>
#include <fstream>

#include <GLApp.h>
#include <bmp.h>
#include <Grid2D.h>

class GLIPApp : public GLApp {
public:
  Image image;
    
  GLIPApp() : GLApp(512, 512, 4, "Image Processing")
  {
  }
  
  void toGrayscale(bool uniform=false) {
    for (uint32_t y = 0; y < image.height; y++) {
      for (uint32_t x = 0; x < image.width; x++) {
        uint8_t red = image.getValue(x, y, 0);
        uint8_t green = image.getValue(x, y, 1);
        uint8_t blue = image.getValue(x, y, 2);
        uint8_t outputValue = 0;
        
        if (uniform) {
          outputValue = red * 0.333f + green * 0.333f + blue * 0.333f;
        } else {
          outputValue = red * 0.299f + green * 0.587f + blue * 0.114f;
        }

        image.setValue(x, y, outputValue);
      }
    }
  }

  void loadImage() {
    try {
      image = BMP::load("lenna.bmp");
    } catch (...) {
      image = Image(512,512);
      for (uint32_t y = 0;y<image.height;++y) {
        for (uint32_t x = 0;x<image.width;++x) {
          image.setNormalizedValue(x,y,0,float(x)/image.width);
          image.setNormalizedValue(x,y,1,float(y)/image.height);
          image.setNormalizedValue(x,y,2,0.5f);
          image.setValue(x,y,3,255);
        }
      }
    }
  }
  
  virtual void init() override {
    loadImage();
  }
      
  virtual void draw() override {
    drawImage(image);
  }
  
  
  std::string toString(bool bSmallTable=true) {
    const std::string lut1{"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "};
    const std::string lut2{"@%#*+=-:. "};
    const std::string& lut = bSmallTable ? lut2 : lut1;
    
    std::stringstream ss;

    // TODO: convert image to ASCII-Art

    return ss.str();
  }
  
  void filter(const Grid2D& filter) {
    // TODO: apply filter to image
    uint32_t filterXMid = filter.getWidth() / 2;
    uint32_t filterYMid = filter.getHeight() / 2;

    for (uint32_t y = 0; y < image.height; y++) {
      for (uint32_t x = 0; x < image.width; x++) {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        for (uint32_t f_y = 0; f_y < filter.getHeight(); f_y++) {
          int64_t filterOnImageY = y + (f_y - filterYMid);
          if (filterOnImageY < 0 || filterOnImageY >= image.height) {
            continue;
          }

          for (uint32_t f_x = 0; f_x < filter.getWidth(); f_x++) {
            int64_t filterOnImageX = x + (f_x - filterXMid);
            if (filterOnImageX < 0 || filterOnImageX >= image.width) {
              continue;
            }

            float filterWeight = filter.getValue(f_x, f_y);
            int16_t weightedRed = image.getValue(filterOnImageX, filterOnImageY, 0) * filterWeight;
            int16_t weightedGreen = image.getValue(filterOnImageX, filterOnImageY, 1) * filterWeight;
            int16_t weightedBlue = image.getValue(filterOnImageX, filterOnImageY, 2) * filterWeight;

            red += weightedRed;
            green += weightedGreen;
            blue += weightedBlue;
          }
        }

        image.setValue(x, y, 0, red);
        image.setValue(x, y, 1, green);
        image.setValue(x, y, 2, blue);
      }
    }
  }
  
  virtual void keyboard(int key, int scancode, int action, int mods) override {
    if (action == GLENV_PRESS) {
      switch (key) {
        case GLENV_KEY_ESCAPE :
          closeWindow();
          break;
        case GLENV_KEY_M :
          {
            Grid2D mean{3,3};
            mean.fill(1.0f/(mean.getHeight()*mean.getWidth()));
            filter(mean);
          }
          break;
        case GLENV_KEY_A :
          filter({3,3, {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1}});
          break;
        case GLENV_KEY_B:
          filter({3,3, {-1,-2,-1,
                         0, 0, 0,
                         1, 2, 1}});
        break;
        case GLENV_KEY_G :
          toGrayscale(false);
          break;
        case GLENV_KEY_H :
          toGrayscale(true);
          break;
        case GLENV_KEY_R :
          loadImage();
          break;
        case GLENV_KEY_T : {
          std::ofstream file{ "ascii.txt" };
          file << toString() << std::endl;
          break;
        }
      }
    }
  }
  
};

#ifdef _WIN32
#include <Windows.h>

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    GLIPApp imageProcessing;
    imageProcessing.run();
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
