#ifndef _COUNT_SKETCH_H
#define _COUNT_SKETCH_H

#include "common.h"
#include "utils.h"

template<typename T, uint32_t d, uint32_t w>
class CountSketch {
  private:
    int depth = d, width = w;
    uint64_t h[d << 1], s[d << 1], n[d << 1];

  public:
    std::vector<std::vector<T>> matrix;

    CountSketch() {
      matrix = std::vector<std::vector<T>>(d, std::vector<T>(width, 0));
      int index = 0;
      for (int i = 0; i < (depth << 1); ++i) {
        h[i] = GenHashSeed(index++);
        s[i] = GenHashSeed(index++);
        n[i] = GenHashSeed(index++);
      }
    }

    uint32_t hash(Key_t key, int line) {
      return (uint32_t)(AwareHash((unsigned char *)key, CHARKEY_LEN + 1, h[line], s[line], n[line]) % width);
    }

    void insert(Key_t key, uint32_t val = 1) {
      uint32_t pos;
      // std::cout << val << std::endl;
      for (int i = 0; i < depth; ++i) {
        pos = hash(key, i);
        matrix[i][pos] += (int32_t)val * ((int32_t)(hash(key, depth + i) & 1) * 2 - 1);
        // std::cout << matrix[i][pos] << std::endl;
        // assert(matrix[i][pos] != 0);
      }
    }

    uint32_t query(Key_t key) {
      std::vector<T> result;
      uint32_t pos;

      for (int i = 0; i < depth; ++i) {
        pos = hash(key, i);
        result.push_back(matrix[i][pos] * ((int32_t)(hash(key, depth + i) & 1) * 2 - 1));
        // std::cout << ((int)(hash(key, depth + i) & 1) * 2 - 1) << std::endl;
      }

      std::sort(result.begin(), result.end());
      // std::cout << result.at(depth / 2) << " " << result.at(depth / 2) + result.at(depth / 2 - 1) << std::endl;
      if (depth & 1) {
        return std::abs(result.at(depth / 2));
      }
      else {
        return std::abs(result.at(depth / 2) + result.at(depth / 2 - 1)) / 2;
      }
    }

    size_t get_memory_usage() {
      return depth * width * sizeof(T);
    }
};

#endif
