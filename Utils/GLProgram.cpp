#include <sstream>
#include <fstream>
#include <iostream>

#include "GLProgram.h"
#include "GLDebug.h"

GLProgram::GLProgram(const GLProgram& other) :
  GLProgram(other.vertexShaderStrings, other.fragmentShaderStrings, other.geometryShaderStrings)
{
}

GLProgram& GLProgram::operator=(const GLProgram& other) {
  GL(glDeleteShader(glVertexShader));
  GL(glDeleteShader(glFragmentShader));
  GL(glDeleteShader(glGeometryShader));
  GL(glDeleteProgram(glProgram));
  programFromVectors(other.vertexShaderStrings, other.fragmentShaderStrings, other.geometryShaderStrings);
  return *this;
}

GLuint GLProgram::createShader(GLenum type, const GLchar** src, GLsizei count) {
	if (count==0) return 0;
	GLuint s = glCreateShader(type); checkAndThrow();
	glShaderSource(s, count, src, NULL); checkAndThrow();
	glCompileShader(s); checkAndThrowShader(s);
	return s;
}

GLProgram::GLProgram(std::vector<std::string> vertexShaderStrings, std::vector<std::string> fragmentShaderStrings, std::vector<std::string> geometryShaderStrings):
  glVertexShader(0),
  glFragmentShader(0),
  glGeometryShader(0),
  glProgram(0),
  vertexShaderStrings(vertexShaderStrings),
  fragmentShaderStrings(fragmentShaderStrings),
  geometryShaderStrings(geometryShaderStrings)
{
  programFromVectors(vertexShaderStrings, fragmentShaderStrings, geometryShaderStrings);
}

GLProgram::~GLProgram() {
	GL(glDeleteShader(glVertexShader));
	GL(glDeleteShader(glFragmentShader));
  GL(glDeleteShader(glGeometryShader));
	GL(glDeleteProgram(glProgram));
}

GLProgram GLProgram::createFromFiles(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	std::vector<std::string> vsTexts;
	for (const std::string& f : vs) {
		vsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> fsTexts;
	for (const std::string& f : fs) {
		fsTexts.push_back(loadFile(f));
	}
	std::vector<std::string> gsTexts;
	for (const std::string& f : gs) {
		if (!f.empty())		
			gsTexts.push_back(loadFile(f));
	}
	return createFromStrings(vsTexts,fsTexts,gsTexts);
}

GLProgram GLProgram::createFromStrings(const std::vector<std::string>& vs, const std::vector<std::string>& fs, const std::vector<std::string>& gs) {
	return {vs,fs,gs};
}

GLProgram GLProgram::createFromFile(const std::string& vs, const std::string& fs, const std::string& gs) {
	return createFromFiles(std::vector<std::string>{vs}, std::vector<std::string>{fs}, std::vector<std::string>{gs});
}

GLProgram GLProgram::createFromString(const std::string& vs, const std::string& fs, const std::string& gs) {
	return createFromStrings(std::vector<std::string>{vs}, std::vector<std::string>{fs}, std::vector<std::string> {gs});
}

std::string GLProgram::loadFile(const std::string& filename) {
	std::ifstream shaderFile{filename};
	if (!shaderFile) {
		throw ProgramException{std::string("Unable to open file ") +  filename};
	}
	std::string str;
	std::string fileContents;
	while (std::getline(shaderFile, str)) {
		fileContents += str + "\n";
	} 
	return fileContents;
}

GLint GLProgram::getAttributeLocation(const std::string& id) const {
  const GLint l = glGetAttribLocation(glProgram, id.c_str());
	checkAndThrow();	
	if(l == -1)
		throw ProgramException{std::string("Can't find attribute ") +  id};	
	return l;
}

GLint GLProgram::getUniformLocation(const std::string& id) const {
	const GLint l = glGetUniformLocation(glProgram, id.c_str());
	checkAndThrow();
	if(l == -1)
		throw ProgramException{std::string("Can't find uniform ") +  id};	
	return l;
}

void GLProgram::enable() const {
	GL(glUseProgram(glProgram));
}

void GLProgram::disable() const {
	GL(glUseProgram(0));
}

void GLProgram::setUniform(GLint id, float value) const {
	GL(glUniform1f(id, value));
}

void GLProgram::setUniform(GLint id, const Vec2& value) const {
  GL(glUniform2fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec3& value) const {
	GL(glUniform3fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec4& value) const {
  GL(glUniform4fv(id, 1, value));
}

void GLProgram::setUniform(GLint id, int value) const {
  GL(glUniform1i(id, value));
}

void GLProgram::setUniform(GLint id, const Vec2i& value) const {
  GL(glUniform2iv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec3i& value) const {
  GL(glUniform3iv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Vec4i& value) const {
  GL(glUniform4iv(id, 1, value));
}

void GLProgram::setUniform(GLint id, const Mat4& value, bool transpose) const {
	// since OpenGL matrices are usuall expected
  // column major but our matrices are row major
  // hence, we invert the transposition flag
  GL(glUniformMatrix4fv(id, 1, !transpose, value));
}

void GLProgram::setUniform(GLint id, const std::vector<float>& value) const {
  GL(glUniform1fv(id, GLsizei(value.size()), value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec2>& value) const {
  GL(glUniform2fv(id, GLsizei(value.size()), (GLfloat*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec3>& value) const {
  GL(glUniform3fv(id, GLsizei(value.size()), (GLfloat*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec4>& value) const {
  GL(glUniform4fv(id, GLsizei(value.size()), (GLfloat*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<int>& value) const {
  GL(glUniform1iv(id, GLsizei(value.size()), (GLint*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec2i>& value) const {
  GL(glUniform2iv(id, GLsizei(value.size()), (GLint*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec3i>& value) const {
  GL(glUniform3iv(id, GLsizei(value.size()), (GLint*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Vec4i>& value) const {
  GL(glUniform4iv(id, GLsizei(value.size()), (GLint*)value.data()));
}

void GLProgram::setUniform(GLint id, const std::vector<Mat4>& value, bool transpose) const {
  // since OpenGL matrices are usuall expected
  // column major but our matrices are row major
  // hence, we invert the transposition flag
  GL(glUniformMatrix4fv(id, GLsizei(value.size()), !transpose, (GLfloat*)value.data()));
}

#ifndef __EMSCRIPTEN__
void GLProgram::setTexture(GLint id, const GLTexture1D& texture, GLenum unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_1D, texture.getId()));
  GL(glUniform1i(id, GLint(unit)));
}
#endif

void GLProgram::setTexture(GLint id, const GLTexture2D& texture, GLenum unit) const {
	GL(glActiveTexture(GL_TEXTURE0 + unit));
	GL(glBindTexture(GL_TEXTURE_2D, texture.getId()));
	GL(glUniform1i(id, GLint(unit)));
}

void GLProgram::setTexture(GLint id, const GLTexture3D& texture, GLenum unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_3D, texture.getId()));
  GL(glUniform1i(id, GLint(unit)));
}

#ifndef __EMSCRIPTEN__
void GLProgram::unsetTexture1D(GLenum unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_1D, 0));
}
#endif

void GLProgram::unsetTexture2D(GLenum unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLProgram::unsetTexture3D(GLenum unit) const {
  GL(glActiveTexture(GL_TEXTURE0 + unit));
  GL(glBindTexture(GL_TEXTURE_3D, 0));
}

void GLProgram::programFromVectors(std::vector<std::string> vs, std::vector<std::string> fs, std::vector<std::string> gs) {
  vertexShaderStrings   = vs;
  fragmentShaderStrings = fs;
  geometryShaderStrings = gs;

  std::vector<const GLchar*> vertexShaderTexts;
  for (const std::string& s : vertexShaderStrings)
   vertexShaderTexts.push_back(s.c_str());

  std::vector<const GLchar*> fragmentShaderTexts;
  for (const std::string& s : fragmentShaderStrings)
   fragmentShaderTexts.push_back(s.c_str());
   
  std::vector<const GLchar*> geometryShaderTexts;
  for (const std::string& s : geometryShaderStrings)
   if (!s.empty())
     geometryShaderTexts.push_back(s.c_str());

  glVertexShader = createShader(GL_VERTEX_SHADER, vertexShaderTexts.data(), GLsizei(vertexShaderTexts.size()));
  glFragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderTexts.data(), GLsizei(fragmentShaderTexts.size()));
  glGeometryShader = createShader(GL_GEOMETRY_SHADER, geometryShaderTexts.data(), GLsizei(geometryShaderTexts.size()));

  glProgram = glCreateProgram(); checkAndThrow();
  if (glVertexShader) {glAttachShader(glProgram, glVertexShader); checkAndThrow();}
  if (glFragmentShader) {glAttachShader(glProgram, glFragmentShader); checkAndThrow();}
  if (glGeometryShader) {glAttachShader(glProgram, glGeometryShader); checkAndThrow();}
  glLinkProgram(glProgram); checkAndThrowProgram(glProgram);
}


void GLProgram::setUniform(const std::string& id, float value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec2& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec3& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec4& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, int value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Vec2i& value) const {
  setUniform(getUniformLocation(id), value);
}

void GLProgram::setUniform(const std::string& id, const Mat4& value, bool transpose) const {
  setUniform(getUniformLocation(id), value, transpose);
}


#ifndef __EMSCRIPTEN__
void GLProgram::setTexture(const std::string& id, const GLTexture1D& texture, GLenum unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}
#endif

void GLProgram::setTexture(const std::string& id, const GLTexture2D& texture, GLenum unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}

void GLProgram::setTexture(const std::string& id, const GLTexture3D& texture, GLenum unit) const {
  setTexture(getUniformLocation(id), texture, unit);
}
