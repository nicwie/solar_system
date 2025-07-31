#ifndef MODEL_HPP
#define MODEL_HPP

#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.hpp"
#include "Mesh.hpp"

#include "stb_image.h"

// static cache so models are only loaded once
static std::vector<Texture> textures_loaded_cache;

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

/**
* @class Model
* @brief Represents a 3D model with one or more meshes
*
* This class uses assimp to load 3D models from files.
* It also handles processing nodes, meshes, materials, and textures.
* Textures can also be added after loading.
*/
class Model {
public:
    /**
    * @brief Constructs a model by loading it from a file
    * @param path file path to 3D model
    * @param flip_uvs Flips texture coordinates vertically
    */
    Model(const std::string& path, bool flip_uvs = true) {
        loadModel(path, flip_uvs);
    }

    // move semantics
    Model(Model && other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;

    // copy
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    /**
    * @brief Draws all meshes in a model
    * @param shader The shader to use for drawing
    */
    void Draw(Shader &shader) {
        for (auto& mesh : m_meshes) {
            mesh.Draw(shader);
        }
    }

    /**
    * @brief Adds a new texture from a file to all meshes in this model
    * @param texturePath The file path to the new texture
    * @param typeName The type name to be used for the shader uniform
    *
    * This is a bit ugly since the textures are added to all meshes,
    * but we are only doing this for earth and do not want to create another
    * class for this.
    *
    * Using this requires a different shader for the specific Model this is used on.
    */
    void addTexture(const std::string& texturePath, const std::string& typeName) {
        // check cache
        for (const auto& loaded_tex : textures_loaded_cache) {
            if (loaded_tex.path == texturePath) {
                for (auto& mesh : m_meshes) {
                    mesh.textures.push_back(loaded_tex);
                }
                return;
            }
        }

        Texture newTexture;
        newTexture.id = TextureFromFile(texturePath.c_str(), m_directory);
        newTexture.type = typeName;
        newTexture.path = texturePath;

        textures_loaded_cache.push_back(newTexture);

        for (auto& mesh : m_meshes) {
            mesh.textures.push_back(newTexture);
        }
    }

private:
    std::vector<Mesh> m_meshes;
    std::string m_directory;

    /**
    * @brief Loads model with Assimp and starts processing
    * @path file path to model.
    * @param modelPath Full path to the parent model file
    */
    void loadModel (const std::string& path, bool flip_uvs) {
        Assimp::Importer importer;
        unsigned int process_flags = aiProcess_Triangulate | aiProcess_GenNormals;
        if (flip_uvs) {
            process_flags |= aiProcess_FlipUVs;
        }

        const aiScene* scene = importer.ReadFile(path, process_flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }

        m_directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene, path);
    }


    /**
    * @brief Recursively process each node form the Assimp scene graph
    * @param node Current Assimp node
    * @param modelPath Full path to the parent model file
    * @param scene The Assimp scene object
    */
    void processNode(aiNode *node, const aiScene *scene, const std::string& modelPath){
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene, modelPath));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, modelPath);
        }
    }

    /**
    * @brief Translate Assimp mesh to our format
    * @param scene The Assimp scene object
    * @param modelPath Full path to the parent model file
    * @return A mesh that can be rendered
    */
    Mesh processMesh(aiMesh *mesh, const aiScene* scene, const std::string& modelPath) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            if (mesh->HasNormals()) {
                vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
            }
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }

        // indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // material
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // diffuse
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene, modelPath);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // specular
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene, modelPath);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(std::move(vertices), std::move(indices), std::move(textures));
    }

    /**
    * @brief Load all textures of type from an Assimp material
    * @param mat The Assimp material
    * @param type The Assimp texture to load
    * @param typeName Specific type name of this texture
    * @param scene The Assimp scene for embedded textures
    * @param modelPath Full path to parent model file, for caching
    * @return A vector of loaded Texture objects
    */
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName,
                                              const aiScene *scene, const std::string& modelPath) {
        std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            const char* rawPath = str.C_Str();

            // create a unique path for the texture cache
            std::string uniquePath;
            if (rawPath[0] == '*') {
                uniquePath = modelPath + rawPath;
            } else {
                // Is an external file => Path is unique
                uniquePath = m_directory + '/' + std::string(rawPath);
            }

            bool skip = false;
            for (const auto& loaded_texture : textures_loaded_cache) {
                if (loaded_texture.path == uniquePath) {
                    textures.push_back(loaded_texture);
                    skip = true;
                    break;
                }
            }

            if (!skip) {
                Texture texture;
                if (rawPath[0] == '*') {
                    int textureIndex = std::stoi(std::string(rawPath).substr(1));
                    const aiTexture* embeddedTexture = scene->mTextures[textureIndex];
                    texture.id = textureFromEmbedded(embeddedTexture);
                } else {
                    texture.id = TextureFromFile(rawPath, m_directory);
                }

                texture.type = typeName;
                texture.path = uniquePath;
                textures.push_back(texture);
                textures_loaded_cache.push_back(texture);
            }
        }
        return textures;
    }

    /**
    * @brief Loads an embedded texture from Assimp memory buffer
    * @param texture Assimp texture object
    * @return OpenGL texture ID
    */
    unsigned int textureFromEmbedded(const aiTexture* texture) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;

        unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(texture->pcData),
                                                    texture->mWidth, &width, &height, &nrComponents, 0);

        if (data) {
            GLenum format;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;
            else {
                stbi_image_free(data);
                glDeleteTextures(1, &textureID);
                return 0;
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cerr << "Failed to load embedded texture." << std::endl;
            glDeleteTextures(1, &textureID);
            return 0;
        }
        return textureID;
    }
};

/**
 * @brief Utility function to load a 2D texture from a file.
 * @param path Path to the texture file.
 * @param directory Directory of the model, for resolving relative paths.
 * @return OpenGL texture ID, or 0 on failure.
 */
inline unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma) {
    std::string filename = std::string(path);
    // If the path is not absolute, prepend directory
    if (filename.find(":/") == std::string::npos && filename.find(":\\") == std::string::npos && filename[0] != '/') {
         filename = directory + '/' + filename;
    }


    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        else {
             stbi_image_free(data);
             glDeleteTextures(1, &textureID);
             return 0;
        }


        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}

#endif // !MODEL_HPP
