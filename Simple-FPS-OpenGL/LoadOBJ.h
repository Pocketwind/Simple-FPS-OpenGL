#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class OBJ {
public:
    float ambient = 0.1f;
    float diffuse = 0.7f;
    float specular = 0.6f;
    float shininess = 100.0f;
    vector<vec3> vertices;
    vector<unsigned int> triconnect;
    vector<vec3> texture;
    vector<unsigned int> tritexture;
    vector<vec3> normals;
    vector<unsigned int> trinormal;
    vector<vec3> trivertex;
    vector<vec3> tritexture_coords;
    vector<vec3> trinormal_coords;
    int polygons;

    OBJ(const string& filename, bool isTexture = false) {
        LoadObjfile(filename, isTexture);
        trivertex = vtoev(vertices, triconnect);
        if (isTexture) {
            tritexture_coords = vtoev(texture, tritexture);
        }
        trinormal_coords = vtoev(normals, trinormal);
        polygons = triconnect.size() / 3;
    }

private:
    void LoadObjfile(const string& filename, bool isTexture) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filename << endl;
			throw exception();
            return;
        }

        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            string prefix;
            iss >> prefix;

            if (prefix == "v") {
                vec3 vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                vertices.push_back(vertex);
            }
            else if (prefix == "vt" && isTexture) {
                vec3 texCoord;
                iss >> texCoord.x >> texCoord.y >> texCoord.z;
                texture.push_back(texCoord);
            }
            else if (prefix == "vn") {
                vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (prefix == "f") {
                string vertex1, vertex2, vertex3;
                iss >> vertex1 >> vertex2 >> vertex3;
                triconnect.push_back(ParseFaceIndex(vertex1, 0));
                triconnect.push_back(ParseFaceIndex(vertex2, 0));
                triconnect.push_back(ParseFaceIndex(vertex3, 0));

                if (isTexture) {
                    tritexture.push_back(ParseFaceIndex(vertex1, 1));
                    tritexture.push_back(ParseFaceIndex(vertex2, 1));
                    tritexture.push_back(ParseFaceIndex(vertex3, 1));
                }

                trinormal.push_back(ParseFaceIndex(vertex1, 2));
                trinormal.push_back(ParseFaceIndex(vertex2, 2));
                trinormal.push_back(ParseFaceIndex(vertex3, 2));
            }
        }
    }

    unsigned int ParseFaceIndex(const string& face, int index) {
        vector<string> tokens;
        stringstream ss(face);
        string token;
        while (getline(ss, token, '/')) {
            tokens.push_back(token);
        }
        if (index < tokens.size() && !tokens[index].empty()) {
            return stoi(tokens[index]) - 1;
        }
        return 0;
    }

    vector<vec3> vtoev(const vector<vec3>& vert, const vector<unsigned int>& conn) {
        vector<vec3> tri;
        for (size_t t = 0; t < conn.size(); t += 3) {
            tri.push_back(vert[conn[t]]);
            tri.push_back(vert[conn[t + 1]]);
            tri.push_back(vert[conn[t + 2]]);
        }
        return tri;
    }
};
