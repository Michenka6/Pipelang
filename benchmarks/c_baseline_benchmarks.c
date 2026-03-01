#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#define NOB_IMPLEMENTATION
#include "../include/nob.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static inline uint64_t ns_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // monotonic = safe for benchmarking
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

uint8_t *load_png(const char *filepath, int *width, int *height)
{
    uint8_t *pixels = stbi_load(filepath, width, height, NULL, 4);

    if (!pixels) {
        fprintf(stderr, "Failed to load PNG '%s': %s\n", filepath, stbi_failure_reason());
        return NULL;
    }

    return pixels;
}

bool write_png(const char *filepath, const uint8_t *pixels, int width, int height)
{
    int stride_bytes = width * 4;

    int ok = stbi_write_png(filepath, width, height, 4, pixels, stride_bytes);

    if (!ok) {
        fprintf(stderr, "Failed to write PNG '%s'\n", filepath);
        return false;
    }
    return true;
}

static inline uint8_t u8_clamp_int(int v)
{
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

void adjust_brightness_add(uint8_t *pixels, int width, int height, int delta)
{
    for (int i = 0; i < width * height * 4; i += 4) {
        pixels[i + 0] = u8_clamp_int((int)pixels[i + 0] + delta);
        pixels[i + 1] = u8_clamp_int((int)pixels[i + 1] + delta);
        pixels[i + 2] = u8_clamp_int((int)pixels[i + 2] + delta);
    }
}

void adjust_brightness_mul(uint8_t *pixels, int width, int height, float factor)
{
    if (factor < 0.0f) factor = 0.0f;
    for (int i = 0; i < width * height * 4; i += 4) {
        int r = (int)lroundf(pixels[i + 0] * factor);
        int g = (int)lroundf(pixels[i + 1] * factor);
        int b = (int)lroundf(pixels[i + 2] * factor);
        pixels[i + 0] = u8_clamp_int(r);
        pixels[i + 1] = u8_clamp_int(g);
        pixels[i + 2] = u8_clamp_int(b);
    }
}

    
void rgb_to_grayscale(uint8_t *pixels, int width, int height)
{
    for(int i = 0; i < width * height * 4; i += 4) {
        uint8_t gray = (uint8_t)pixels[i] * 0.299f + pixels[i+1] * 0.587f + pixels[i+2] * 0.114f;
        pixels[i]   = gray;
        pixels[i+1] = gray;
        pixels[i+2] = gray;
    }
}

void adjust_contrast_mul(uint8_t *pixels, int width, int height, float factor)
{
    for (int i = 0; i < width * height * 4; i += 4) {
        for (int c = 0; c < 3; c++) {
            float x = (float)pixels[i + c];
            float y = (x - 128.0f) * factor + 128.0f;
            int yi = (int)lroundf(y);
            pixels[i + c] = u8_clamp_int(yi);
        }
    }
}
void adjust_contrast_int(uint8_t *pixels, int width, int height, int contrast)
{
    if (contrast < -255) contrast = -255;
    if (contrast >  255) contrast =  255;

    float c = (float)contrast;
    float factor = (259.0f * (c + 255.0f)) / (255.0f * (259.0f - c));

    adjust_contrast_mul(pixels, width, height, factor);
}

void adjust_gamma(uint8_t *pixels, int width, int height, float gamma)
{
    if (gamma <= 0.0f) gamma = 1.0f;

    uint8_t lut[256];

    for (int i = 0; i < 256; i++) {
        float normalized = i / 255.0f;
        float corrected  = powf(normalized, gamma);
        lut[i] = (uint8_t)(corrected * 255.0f + 0.5f);
    }

    size_t n = (size_t)width * (size_t)height * 4;

    for (size_t i = 0; i < n; i += 4) {
        pixels[i + 0] = lut[pixels[i + 0]];
        pixels[i + 1] = lut[pixels[i + 1]];
        pixels[i + 2] = lut[pixels[i + 2]];
    }
}

void binarize_rgba(uint8_t *pixels, int width, int height, uint8_t threshold)
{
    for (int i = 0; i < width * height * 4; i += 4) {
        uint8_t r = pixels[i + 0];
        uint8_t g = pixels[i + 1];
        uint8_t b = pixels[i + 2];

        uint8_t gray = (uint8_t)((77u*r + 150u*g + 29u*b) >> 8);

        uint8_t bw = (gray >= threshold) ? 255 : 0;

        pixels[i + 0] = bw;
        pixels[i + 1] = bw;
        pixels[i + 2] = bw;
    }
}

#define WIDTH 3840
#define HEIGHT 2160 
int main(void)
{
    const char *parent = "benchmarks/images";
    Nob_File_Paths children = {0};
    bool images_read = nob_read_entire_dir(parent, &children);
    if (!images_read) return -1;

    uint8_t *pixels = malloc(WIDTH*HEIGHT*4);
    for(size_t i = 2; i < children.count; i += 1) {
        char filepath[100];
        sprintf(filepath, "benchmarks/images/%s", children.items[i]);
        int width; int height;

        memset(pixels, 0, WIDTH*HEIGHT*4);
        uint8_t *original_pixels = load_png(filepath, &width, &height);
        assert(width == WIDTH);
        assert(height == HEIGHT);

        if (1) { // RGB_to grayscale
            memcpy(pixels, original_pixels, WIDTH*HEIGHT*4);

            uint64_t t0 = ns_now();
            rgb_to_grayscale(pixels, WIDTH, HEIGHT);
            uint64_t t1 = ns_now();

            uint64_t dt = t1 - t0;
            printf("grayscale took %llu ns (%.6f ms)\n", (unsigned long long)dt, (double)dt / 1e6);

            char new_filepath[100];
            sprintf(new_filepath, "benchmarks/produced_images/grayscale/%s", children.items[i]);
            bool wrote_img = write_png(new_filepath, pixels, WIDTH, HEIGHT);
            if (wrote_img) printf("Produced new image : \"%s\"\n", new_filepath);
        }

        if (1) { // Brightness adjustment
            memcpy(pixels, original_pixels, WIDTH*HEIGHT*4);

            uint64_t t0 = ns_now();
            adjust_brightness_mul(pixels, WIDTH, HEIGHT, 0.5);
            uint64_t t1 = ns_now();

            uint64_t dt = t1 - t0;
            printf("brightness took %llu ns (%.6f ms)\n", (unsigned long long)dt, (double)dt / 1e6);

            char new_filepath[100];
            sprintf(new_filepath, "benchmarks/produced_images/brightness/%s", children.items[i]);
            bool wrote_img = write_png(new_filepath, pixels, WIDTH, HEIGHT);
            if (wrote_img) printf("Produced new image : \"%s\"\n", new_filepath);
        }

        if (1) { // Contrast adjustment
            memcpy(pixels, original_pixels, WIDTH*HEIGHT*4);

            uint64_t t0 = ns_now();
            adjust_contrast_int(pixels, WIDTH, HEIGHT, 100);
            uint64_t t1 = ns_now();

            uint64_t dt = t1 - t0;
            printf("contrast took %llu ns (%.6f ms)\n", (unsigned long long)dt, (double)dt / 1e6);

            char new_filepath[100];
            sprintf(new_filepath, "benchmarks/produced_images/contrast/%s", children.items[i]);
            bool wrote_img = write_png(new_filepath, pixels, WIDTH, HEIGHT);
            if (wrote_img) printf("Produced new image : \"%s\"\n", new_filepath);
        }

        if (1) { // Gamma adjustment
            memcpy(pixels, original_pixels, WIDTH*HEIGHT*4);

            uint64_t t0 = ns_now();
            adjust_gamma(pixels, WIDTH, HEIGHT, 2.2);
            uint64_t t1 = ns_now();

            uint64_t dt = t1 - t0;
            printf("gamma took %llu ns (%.6f ms)\n", (unsigned long long)dt, (double)dt / 1e6);

            char new_filepath[100];
            sprintf(new_filepath, "benchmarks/produced_images/gamma/%s", children.items[i]);
            bool wrote_img = write_png(new_filepath, pixels, WIDTH, HEIGHT);
            if (wrote_img) printf("Produced new image : \"%s\"\n", new_filepath);
        }

        if (1) { // Binarization
            memcpy(pixels, original_pixels, WIDTH*HEIGHT*4);

            uint8_t threshold = 128;
            uint64_t t0 = ns_now();
            binarize_rgba(pixels, WIDTH, HEIGHT, threshold);
            uint64_t t1 = ns_now();

            uint64_t dt = t1 - t0;
            printf("binarize took %llu ns (%.6f ms)\n", (unsigned long long)dt, (double)dt / 1e6);

            char new_filepath[100];
            sprintf(new_filepath, "benchmarks/produced_images/binarization/%s", children.items[i]);
            bool wrote_img = write_png(new_filepath, pixels, WIDTH, HEIGHT);
            if (wrote_img) printf("Produced new image : \"%s\"\n", new_filepath);
        }
    }
    return 0;
}
