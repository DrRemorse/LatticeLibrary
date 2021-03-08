#include <library/opengl/texture_buffer.hpp>
#include <library/opengl/texture.hpp>
#include <library/opengl/opengl.hpp>

namespace library
{
  BufferTexture::BufferTexture(size_t capacity, GLenum format)
  {
    glGenTextures(1, &this->m_texture);
    Texture::raw_bind(0, GL_TEXTURE_BUFFER, this->m_texture);
    // create buffer (with capacity hint)
    glGenBuffers(1, &this->m_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, this->m_buffer);
    glBufferData(GL_TEXTURE_BUFFER, capacity, nullptr, GL_STREAM_DRAW_ARB);
    // create binding
    glTexBuffer(GL_TEXTURE_BUFFER, format, this->m_buffer);
  }

  void BufferTexture::bind(GLenum unit)
  {
    Texture::raw_bind(unit, GL_TEXTURE_BUFFER, this->m_texture);
  }

  void BufferTexture::upload(const void* data, size_t len)
  {
    glBindBuffer(GL_TEXTURE_BUFFER, this->m_buffer);
    glBufferData(GL_TEXTURE_BUFFER, len, data, GL_STREAM_DRAW_ARB);
  }
}
