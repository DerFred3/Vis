#include "MC.h"
#include "MC.inl"

Isosurface::Isosurface(const Volume& volume, uint8_t isovalue) {
  // TODO: compute isosurface using the mc-algorithm and store
  //       the mesh in the vertices vector
  for (size_t z = 0; z < volume.depth - 1; z++) { // depth
    for (size_t y = 0; y < volume.height - 1; y++) { // height
      for (size_t x = 0; x < volume.width - 1; x++) { // width
        float voxel_data[] = {
          volume.data[x+0 + (y+0) * volume.width + (z+0) * volume.width * volume.height],
          volume.data[x+1 + (y+0) * volume.width + (z+0) * volume.width * volume.height],
          volume.data[x+0 + (y+1) * volume.width + (z+0) * volume.width * volume.height],
          volume.data[x+1 + (y+1) * volume.width + (z+0) * volume.width * volume.height],
          volume.data[x+0 + (y+0) * volume.width + (z+1) * volume.width * volume.height],
          volume.data[x+1 + (y+0) * volume.width + (z+1) * volume.width * volume.height],
          volume.data[x+0 + (y+1) * volume.width + (z+1) * volume.width * volume.height],
          volume.data[x+1 + (y+1) * volume.width + (z+1) * volume.width * volume.height],
        };

        Vec3 voxel_normals[] = {
          volume.normals[x+0 + (y+0) * volume.width + (z+0) * volume.width * volume.height],
          volume.normals[x+1 + (y+0) * volume.width + (z+0) * volume.width * volume.height],
          volume.normals[x+0 + (y+1) * volume.width + (z+0) * volume.width * volume.height],
          volume.normals[x+1 + (y+1) * volume.width + (z+0) * volume.width * volume.height],
          volume.normals[x+0 + (y+0) * volume.width + (z+1) * volume.width * volume.height],
          volume.normals[x+1 + (y+0) * volume.width + (z+1) * volume.width * volume.height],
          volume.normals[x+0 + (y+1) * volume.width + (z+1) * volume.width * volume.height],
          volume.normals[x+1 + (y+1) * volume.width + (z+1) * volume.width * volume.height],
        };
        
        uint8_t oddOnes = 0;
        for (int i = 0; i < 8; i++) {
          if (voxel_data[i] >= isovalue) oddOnes |= 1 << i;
        }
        
        uint8_t edge_idx = 0;
        for (int i = 0; i < 6; i++) {
          for (int j = 0; j < 3; j++) {
            edge_idx = trisTable[oddOnes][i * 3 + j];
            if (edge_idx == N_E) break;
            
            uint8_t v1_idx = edgeToVertexTable[edge_idx][0];
            uint8_t v2_idx = edgeToVertexTable[edge_idx][1];

            float weight;
            float w1;
            float w2;
            w1 = isovalue / volume.data[v1_idx];
            w2 = isovalue / volume.data[v2_idx];

            if (w1 >= 0.0 && w1 <= 1.0) weight = w1;
            else weight = 1.0 - w2;

            Vertex newVertex = Vertex();
            newVertex.position = (Vec3(x, y, z) + vertexPosTable[v1_idx]) * weight + (Vec3(x, y, z) + vertexPosTable[v2_idx]) * (1.0 - weight);
            newVertex.position.x -= (volume.width / 2);
            newVertex.position.x /= volume.width;
            newVertex.position.y -= (volume.height / 2);
            newVertex.position.y /= volume.height;
            newVertex.position.z -= (volume.depth / 2);
            newVertex.position.z /= volume.depth;

            newVertex.normal = voxel_normals[v1_idx] * weight + voxel_normals[v2_idx] * (1.0 - weight);
            
            vertices.push_back(newVertex);
          }
          if (edge_idx == N_E) break;
        }
      }
    }
  }
}