#include "Shaders.h"

GLuint compileShaderShaders(const char* shaderStr, GLenum shaderType, const char* name)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderStr, NULL);
    glCompileShader(shader);
    GLint res;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE)
    {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
        char* buff = new char[res];
        glGetShaderInfoLog(shader, res, &res, buff);
        fprintf(stderr, "Error Shader %s: %s", name, buff);
        delete[] buff;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void linkProgramShaders(GLuint program)
{
    glLinkProgram(program);
    GLint res;
    glGetProgramiv(program, GL_LINK_STATUS, &res);
    if (res == GL_FALSE)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
        char* buff = new char[res];
        glGetProgramInfoLog(program, res, &res, buff);
        fprintf(stderr, "Error Link: %s", buff);
        delete[] buff;
    }
}

Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, char* texturePath, bool fliped)
{
    vertexShaderSource = GetShaderFromPath(vertexShaderPath);
    geometryShaderSource = GetShaderFromPath(geometryShaderPath);
    fragmentShaderSource = GetShaderFromPath(fragmentShaderPath);

    stbi_set_flip_vertically_on_load(fliped);
    textureData = stbi_load(texturePath, &width, &height, &numberOfColorChannels, 0);
}

Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, char* texturePath, bool fliped, std::vector<std::string> faces)
{
    vertexShaderSource = GetShaderFromPath(vertexShaderPath);
    geometryShaderSource = GetShaderFromPath(geometryShaderPath);
    fragmentShaderSource = GetShaderFromPath(fragmentShaderPath);

    stbi_set_flip_vertically_on_load(fliped);

    textureID = LoadCubemap(faces);
}

Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath, char* texturePath, bool fliped)
{
    vertexShaderSource = GetShaderFromPath(vertexShaderPath);
    fragmentShaderSource = GetShaderFromPath(fragmentShaderPath);

    stbi_set_flip_vertically_on_load(fliped);
    textureData = stbi_load(texturePath, &width, &height, &numberOfColorChannels, 0);
}

Shader::~Shader() {}

std::string Shader::GetShaderFromPath(std::string fragmentPath)
{
    std::string line;
    std::string result;

    // Read File
    std::ifstream file(fragmentPath);
    if (file.is_open())
    {
        std::stringstream shaderData;
        shaderData << file.rdbuf();

        result = shaderData.str();

        file.close();
    }
    else
    {
        std::cout << "Unable to open fragmentPath file" << std::endl;
        return "";
    }

    return result;
}

void Shader::CreateAllShaders()
{
    char* vertexS = new char[vertexShaderSource.size() + 1];
    std::copy(vertexShaderSource.begin(), vertexShaderSource.end(), vertexS);
    vertexS[vertexShaderSource.size()] = '\0';

    char* geometryS = new char[geometryShaderSource.size() + 1];
    std::copy(geometryShaderSource.begin(), geometryShaderSource.end(), geometryS);
    geometryS[geometryShaderSource.size()] = '\0';

    char* fragmentS = new char[fragmentShaderSource.size() + 1];
    std::copy(fragmentShaderSource.begin(), fragmentShaderSource.end(), fragmentS);
    fragmentS[fragmentShaderSource.size()] = '\0';

    //Crear ID Shader 
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Cargar datos del Shader en la ID
    glShaderSource(vertex_shader, 1, &vertexS, NULL);
    glShaderSource(geometry_shader, 1, &geometryS, NULL);
    glShaderSource(fragment_shader, 1, &fragmentS, NULL);

    //Operar con el Shader -> Pilla la string que te paso y traducelo a binario
    compileShaderShaders(&vertexS[0], GL_VERTEX_SHADER, "vertex");
    compileShaderShaders(&geometryS[0], GL_GEOMETRY_SHADER, "geometry");
    compileShaderShaders(&fragmentS[0], GL_FRAGMENT_SHADER, "fragment");

    //Crear programa y enlazarlo con los Shaders (Operaciones Bind())
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, geometry_shader);
    glAttachShader(program, fragment_shader);

    linkProgramShaders(program);

    // Destroy
    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);

    delete[] vertexS;
    delete[] geometryS;
    delete[] fragmentS;
}

void Shader::UseProgram()
{
    glUseProgram(program);
}

void Shader::DeleteProgram()
{
    glDeleteProgram(program);
}

GLuint Shader::GetProgram()
{
    return program;
}

void Shader::GenerateTexture()
{
    // TEXTURE
    glGenTextures(1, &textureID); // Create texture handle
    glBindTexture(GL_TEXTURE_2D, textureID); // Bind
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData); // Load data
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Configure parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Configure parameters
    stbi_image_free(textureData);
    //
}

void Shader::ActivateTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Shader::ActivateCubemapTexture()
{
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void Shader::ActivateTexture(int newTex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newTex);
}

void Shader::GenerateFramebufferTexture()
{
    // Setup FBO texture
    glGenFramebuffers(1, &fbo);
    
    // Create texture exactly as before:
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 800, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // If we need a depth or stencil buffer, we do it here
    // We bind texture (or renderbuffer) to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    // If we had depth or stencil, we would do it here.
}

GLuint Shader::LoadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (int i = 0; i < faces.size(); i++)
    {
        textureData = stbi_load(faces[i].c_str(), &width, &height, &numberOfColorChannels, 0);

        if (textureData)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
            stbi_image_free(textureData);
        }
        else
        {
            printf("ERROR - failed to load path");
            stbi_image_free(textureData);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

GLuint Shader::GetUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(program, uniformName);
}

void Shader::SetUniformFloat(char* uniformName, float value)
{
    glUniform1f(GetUniformLocation(uniformName), value);
}

void Shader::SetUniformInt(char* uniformName, int value)
{
    glUniform1i(GetUniformLocation(uniformName), value);
}

void Shader::SetUniformVector3(char* uniformName, glm::vec3 value)
{
    glUniform3f(GetUniformLocation(uniformName), value.x, value.y, value.z);
}

void Shader::SetUniformVector4(char* uniformName, glm::vec3 value)
{
    glUniform4f(GetUniformLocation(uniformName), value.x, value.y, value.z, 1.0f);
}

void Shader::SetUniformMatrix4(const char* uniformName, glm::mat4 value)
{
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, glm::value_ptr(value));
}

int Shader::GetTextureWidth()
{
    return width;
}

int Shader::GetTextureHeight()
{
    return height;
}
