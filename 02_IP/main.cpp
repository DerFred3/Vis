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

    for (int64_t y = image.height - 1; y >= 0; y--) {
      for (uint32_t x = 0; x < image.width; x++) {
        uint8_t value = image.getValue(x, y, 0) * 0.299f + image.getValue(x, y, 1) * 0.587f + image.getValue(x, y, 2) * 0.114f;
        uint8_t valueBasedIndex = ((1 - (float)value / 255.0f)) * lut.length();
        ss.put(lut[valueBasedIndex]);
      }
      ss.put('\n');
    }
    ss.put('\n');
    return ss.str();
  }
  
  void filter(const Grid2D& filter) {
    for (uint8_t component = 0; component < 3; component++) {
      float lowestValue = 255;
      float highestValue = 0;
      float values[image.width * image.height] = {0};
      for (uint32_t y = 0; y < image.height; y++) {
        for (uint32_t x = 0; x < image.width; x++) {
          float value = 0;
          
          if (x == 0 || y == 0 || x == image.width - 1 || y == image.height - 1) {
            value = image.getValue(x, y, component);
          } else {
            value = image.getValue(x - 1, y - 1, component) * filter.getValue(0, 0) +
                    image.getValue(x, y - 1, component) * filter.getValue(1, 0) +
                    image.getValue(x + 1, y - 1, component) * filter.getValue(2, 0) +
                    image.getValue(x - 1, y, component) * filter.getValue(0, 1) +
                    image.getValue(x, y, component) * filter.getValue(1, 1) +
                    image.getValue(x + 1, y, component) * filter.getValue(2, 1) +
                    image.getValue(x - 1, y + 1, component) * filter.getValue(0, 2) +
                    image.getValue(x, y + 1, component) * filter.getValue(1, 2) +
                    image.getValue(x + 1, y + 1, component) * filter.getValue(2, 2);

            if (value > highestValue) highestValue = value;
            else if (value < lowestValue) lowestValue = value;
          }

          values[x + y * image.width] = value;
        }
      }

      for (uint32_t y = 0; y < image.height; y++) {
        for (uint32_t x = 0; x < image.width; x++) {
          if (highestValue > 255 || lowestValue < 0) {
            image.setNormalizedValue(x, y, component, values[x + y * image.width] / (highestValue - lowestValue));
          } else {
            image.setValue(x, y, component, values[x + y * image.width]);
          }
        }
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
