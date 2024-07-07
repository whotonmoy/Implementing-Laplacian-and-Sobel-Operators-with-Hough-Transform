#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

const int width = 280;
const int height = 280;

// Function to read raw image data from a file
void readRawImage(const string& filename, unsigned char* data) {
    ifstream file(filename, ios::binary);
    file.read(reinterpret_cast<char*>(data), width * height);
    file.close();
}

// Function to write image data to a raw file
void writeRawImage(const string& filename, const unsigned char* data) {
    ofstream file(filename, ios::binary);
    file.write(reinterpret_cast<const char*>(data), width * height);
    file.close();
}

// Function to apply Laplacian Operator and find zero-crossing points
void laplacianOperator(const unsigned char* input, unsigned char* output) {
    for (int i = 1; i < height - 1; ++i) {
        for (int j = 1; j < width - 1; ++j) {
            int sum = -4 * input[i * width + j];
            sum += input[(i - 1) * width + j] + input[(i + 1) * width + j] + input[i * width + j - 1] + input[i * width + j + 1];
            output[i * width + j] = (sum < 0) ? 0 : ((sum > 255) ? 255 : sum);
        }
    }
}

// Function to apply Sobel Operator and obtain gradient at all pixels
void sobelOperator(const unsigned char* input, unsigned char* output) {
    // Sobel operator masks
    int sobelX[3][3] = { { -1, 0, 1 },{ -2, 0, 2 },{ -1, 0, 1 } };
    int sobelY[3][3] = { { -1, -2, -1 },{ 0, 0, 0 },{ 1, 2, 1 } };

    for (int i = 1; i < height - 1; ++i) {
        for (int j = 1; j < width - 1; ++j) {
            int sumX = 0;
            int sumY = 0;

            // Apply Sobel operator
            for (int u = -1; u <= 1; ++u) {
                for (int v = -1; v <= 1; ++v) {
                    sumX += sobelX[u + 1][v + 1] * input[(i + u) * width + j + v];
                    sumY += sobelY[u + 1][v + 1] * input[(i + u) * width + j + v];
                }
            }

            // Compute gradient magnitude
            int magnitude = sqrt(sumX * sumX + sumY * sumY);
            output[i * width + j] = (magnitude > 255) ? 255 : magnitude;
        }
    }
}

// Function to perform Hough transform to extract lines
void houghTransform(const unsigned char* edgeImage, vector<pair<int, int>>& lines) {
    const double pi = 3.14159265358979323846;
    const int maxRho = static_cast<int>(sqrt(width * width + height * height));
    const int thetaBins = 180;
    const int rhoBins = 2 * maxRho;

    vector<vector<int>> accumulator(rhoBins, vector<int>(thetaBins, 0));

    // Populate the accumulator
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (edgeImage[i * width + j] > 0) {
                for (int theta = 0; theta < thetaBins; ++theta) {
                    double rho = j * cos(theta * pi / 180) + i * sin(theta * pi / 180);
                    int rhoIndex = static_cast<int>(rho + maxRho);
                    accumulator[rhoIndex][theta]++;
                }
            }
        }
    }

    // Find lines in accumulator
    const int threshold = 100;
    for (int rhoIndex = 0; rhoIndex < rhoBins; ++rhoIndex) {
        for (int theta = 0; theta < thetaBins; ++theta) {
            if (accumulator[rhoIndex][theta] > threshold) {
                lines.push_back(make_pair(rhoIndex - maxRho, theta));
            }
        }
    }
}

int main() {
    unsigned char building[width * height];
    readRawImage("building.raw", building);

    // Save the original building image
    writeRawImage("original_building.raw", building);

    // Step 1: Apply Laplacian Operator to find zero-crossing points
    unsigned char laplacianOutput[width * height];
    laplacianOperator(building, laplacianOutput);
    writeRawImage("laplacian_output.raw", laplacianOutput);

    // Step 2: Apply Sobel Operator to obtain gradient at all pixels
    unsigned char gradientOutput[width * height];
    sobelOperator(building, gradientOutput);
    writeRawImage("sobel_output.raw", gradientOutput);

    // Step 3: Generate an edge image
    double gradientThreshold = 50.0;
    unsigned char edgeImage[width * height];
    for (int i = 0; i < width * height; ++i) {
        edgeImage[i] = (laplacianOutput[i] > 0 && gradientOutput[i] >= gradientThreshold) ? 255 : 0;
    }
    writeRawImage("edge_image.raw", edgeImage);

    // Step 4: Implement Hough transform algorithm
    vector<pair<int, int>> lines;
    houghTransform(edgeImage, lines);

    // Extract the three longest linear structures (simple sorting for demonstration)
    sort(lines.begin(), lines.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        return a.first > b.first;
        });

    return 0;
}
