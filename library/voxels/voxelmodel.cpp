#include <library/voxels/voxelmodel.hpp>

#include <library/opengl/opengl.hpp>
#include <library/voxels/voxelizer.hpp>

namespace library
{
  VoxelModel::VoxelModel(GLsizei vertices, GLvoid* data)
	{
		vao.begin(sizeof(XModel::xvertex_t), vertices, data);
		vao.attrib(0, 3, GL_FLOAT,         GL_FALSE, 0);
		vao.attrib(1, 4, GL_BYTE,          GL_TRUE, 12);
		vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16);
		vao.end();
	}

	VoxelModel::VoxelModel(const VoxelModel& vx)
	{
		this->vao = vx.vao;
	}

	void VoxelModel::render()
	{
		vao.render(GL_QUADS);
	}
	void VoxelModel::render(GLint mode)
	{
		vao.render(mode);
	}

}
