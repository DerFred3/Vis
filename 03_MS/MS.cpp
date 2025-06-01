#include<algorithm>

#include "MS.h"
#include "MS.inl"

uint32_t TransCoord(float f, uint32_t a) {
  return (f + 1.0f) / 2.0f * a;
}

Vec2 CalcIntersectPoint(Image image, uint8_t isovalue, Vec2 a, Vec2 b) {
  uint8_t valA = image.getValue(TransCoord(a.x, image.width), TransCoord(a.y, image.height), 0);
  uint8_t valB = image.getValue(TransCoord(b.x, image.width), TransCoord(b.y, image.height), 0);

  Vec2 intersect;

  if (valA >= isovalue) {
    float isoScalar = ((float)(isovalue - valB)) / ((float)(valA - valB));
    intersect = b + Vec2(isoScalar * (a.x - b.x), isoScalar * (a.y - b.y));
  } else {
    float isoScalar = ((float)(isovalue - valA)) / ((float)(valB - valA));

    intersect =  a + Vec2(isoScalar * (b.x - a.x), isoScalar * (b.y - a.y));
  }

  // uint8_t intersectVal = image.getValue(intersect.x * image.width + image.width, intersect.y * image.height + image.height, 0);
  // printf("a: (%f,%f); b: (%f,%f)\n", a.x, a.y, b.x, b.y);
  // printf("a_val: %d; b_val %d\n", valA, valB);
  // printf("intersect: (%f,%f)\n", intersect.x, intersect.y);
  // printf("iso: %d, intersect: %d\n", isovalue, intersectVal);
  // printf("\n=============\n\n");

  return intersect;
}

Isoline::Isoline(const Image& image, uint8_t isovalue,
                 bool useAsymptoticDecider) {
  // TODO: compute isoline using the ms-algorithm and
  //       store the 2D points of the line list in the
  //       member variable "vertices"
  // two vertices per line; how does asymptotic decider work?

  const float deltaX = 1.0f / image.width;
  const float deltaY = 1.0f / image.height;

  for (float y = -1.0f + 2.0f / image.height; y <= 1.0f; y += 2.0f / image.height) {
    for (float x = -1.0f + 2.0f / image.width; x <= 1.0f; x += 2.0f / image.width) {      
      Vec2 upLeftPos = Vec2(x - deltaX, y + deltaY);
      Vec2 upRightPos = Vec2(x + deltaX, y + deltaY);
      Vec2 downLeftPos = Vec2(x - deltaX, y - deltaY);
      Vec2 downRightPos = Vec2(x + deltaX, y - deltaY);

      uint8_t upLeftVal = image.getValue(TransCoord(upLeftPos.x, image.width), TransCoord(upLeftPos.y, image.height), 0);
      uint8_t upRightVal = image.getValue(TransCoord(upRightPos.x, image.width), TransCoord(upRightPos.y, image.height), 0);
      uint8_t downLeftVal = image.getValue(TransCoord(downLeftPos.x, image.width), TransCoord(downLeftPos.y, image.height), 0);
      uint8_t downRightVal = image.getValue(TransCoord(downRightPos.x, image.width), TransCoord(downRightPos.y, image.height), 0);

      int8_t upLeftIso = upLeftVal < isovalue ? -1 : 1;
      int8_t upRightIso = upRightVal < isovalue ? -1 : 1;
      int8_t downLeftIso = downLeftVal < isovalue ? -1 : 1;
      int8_t downRightIso = downRightVal < isovalue ? -1 : 1;

      // printf("%d %d\n%d %d\n\n", upLeftVal, upRightVal, downLeftVal, downRightVal);

      int8_t sum = upLeftIso + upRightIso + downLeftIso + downRightIso;

      // printf("%d\n", sum);

      // None off -> easiest case (skip)
      if (sum == -4 || sum == 4) continue;
      
      if (sum == 2 || sum == -2) {
        // One off -> draw isoline `around`
        if (upLeftIso != upRightIso && upLeftIso != downLeftIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos));
        } else if (upRightIso != upLeftIso && upRightIso != downRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, upLeftPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, downRightPos));
        } else if (downLeftIso != upLeftIso && downLeftIso != downRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, upLeftPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos));
        } else {
          vertices.push_back(CalcIntersectPoint(image, isovalue, downRightPos, upRightPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downRightPos, downLeftPos));
        }
      } else {
        // diagonal or algined case -> decide further
        if (upLeftIso == upRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, downRightPos));
        } else if (upLeftIso == downLeftIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos));
        } else {
          // Look at midpoint and decide what to do
          int8_t isoMidVal = ((float)(upLeftVal + upRightVal + downLeftVal + downRightVal) * 0.25f) < isovalue ? -1 : 1;

          Vec2 topIntersect, rightIntersect, botIntersect, leftIntersect;
          topIntersect = CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos);
          rightIntersect = CalcIntersectPoint(image, isovalue, upRightPos, downRightPos);
          botIntersect = CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos);
          leftIntersect = CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos);

          if (useAsymptoticDecider) {
            Vec2 deciderPoint = (downLeftPos * upRightPos - downRightPos * upLeftPos) / (upRightPos + downLeftPos - downRightPos - upLeftPos);

            if (leftIntersect.y < deciderPoint.y) {
              vertices.push_back(leftIntersect);
              vertices.push_back(botIntersect);
              vertices.push_back(rightIntersect);
              vertices.push_back(topIntersect);
            } else {
              vertices.push_back(leftIntersect);
              vertices.push_back(topIntersect);
              vertices.push_back(rightIntersect);
              vertices.push_back(botIntersect);
            }
          } else {
            if (isoMidVal == upLeftIso && isoMidVal == downRightIso) {
              vertices.push_back(topIntersect);
              vertices.push_back(rightIntersect);
              vertices.push_back(leftIntersect);
              vertices.push_back(botIntersect);
            } else if (isoMidVal == upRightIso && isoMidVal == downLeftIso) {
              vertices.push_back(topIntersect);
              vertices.push_back(leftIntersect);
              vertices.push_back(rightIntersect);
              vertices.push_back(botIntersect);
            }
          }
        }
      }
    }
  }
}