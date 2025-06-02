#include<algorithm>

#include "MS.h"
#include "MS.inl"

uint32_t TransCoord(float f, uint32_t a) {
  return (f + 1.0f) / 2.0f * a;
}

Vec2 CalcIntersectPoint(Image image, uint8_t isovalue, Vec2 a, Vec2 b, uint8_t valA, uint8_t valB) {
  if (valA >= isovalue) {
    float isoScalar = ((float)(isovalue - valB)) / ((float)(valA - valB));
    
    return b + Vec2(isoScalar * (a.x - b.x), isoScalar * (a.y - b.y));
  } else {
    float isoScalar = ((float)(isovalue - valA)) / ((float)(valB - valA));

    return a + Vec2(isoScalar * (b.x - a.x), isoScalar * (b.y - a.y));
  }
}

Isoline::Isoline(const Image& image, uint8_t isovalue,
                 bool useAsymptoticDecider) {
  // TODO: compute isoline using the ms-algorithm and
  //       store the 2D points of the line list in the
  //       member variable "vertices"
  // two vertices per line; how does asymptotic decider work?

  const float deltaX = 1.0f / image.width;
  const float deltaY = 1.0f / image.height;

  for (float y = -1.0f + 2.0f * deltaY; y <= 1.0f; y += 2.0f * deltaY) {
    for (float x = -1.0f + 2.0f * deltaX; x <= 1.0f; x += 2.0f * deltaX) {      
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

      int8_t sum = upLeftIso + upRightIso + downLeftIso + downRightIso;

      // None off -> easiest case (skip)
      if (sum == -4 || sum == 4) continue;
      
      if (sum == 2 || sum == -2) {
        // One off -> draw isoline `around`
        if (upLeftIso != upRightIso && upLeftIso != downLeftIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos, upLeftVal, upRightVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos, upLeftVal, downLeftVal));
        } else if (upRightIso != upLeftIso && upRightIso != downRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, upLeftPos, upRightVal, upLeftVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, downRightPos, upRightVal, downRightVal));
        } else if (downLeftIso != upLeftIso && downLeftIso != downRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, upLeftPos, downLeftVal, upLeftVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos, downLeftVal, downRightVal));
        } else {
          vertices.push_back(CalcIntersectPoint(image, isovalue, downRightPos, upRightPos, downRightVal, upRightVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downRightPos, downLeftPos, downRightVal, downLeftVal));
        }
      } else {
        // diagonal or algined case -> decide further
        if (upLeftIso == upRightIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos, upLeftVal, downLeftVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, upRightPos, downRightPos, upRightVal, downRightVal));
        } else if (upLeftIso == downLeftIso) {
          vertices.push_back(CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos, upLeftVal, upRightVal));
          vertices.push_back(CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos, downLeftVal, downRightVal));
        } else {
          // Look at midpoint and decide what to do
          int8_t midIso = ((upLeftVal + upRightVal + downLeftVal + downRightVal) * 0.25f) < isovalue ? -1 : 1;

          Vec2 topIntersect, rightIntersect, botIntersect, leftIntersect;
          topIntersect = CalcIntersectPoint(image, isovalue, upLeftPos, upRightPos, upLeftVal, upRightVal);
          rightIntersect = CalcIntersectPoint(image, isovalue, upRightPos, downRightPos, upRightVal, downRightVal);
          botIntersect = CalcIntersectPoint(image, isovalue, downLeftPos, downRightPos, downLeftVal, downRightVal);
          leftIntersect = CalcIntersectPoint(image, isovalue, upLeftPos, downLeftPos, upLeftVal, downLeftVal);

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
            if (midIso == upLeftIso && midIso == downRightIso) {
              vertices.push_back(topIntersect);
              vertices.push_back(rightIntersect);
              vertices.push_back(leftIntersect);
              vertices.push_back(botIntersect);
            } else {
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