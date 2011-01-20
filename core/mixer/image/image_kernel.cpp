#include "../../StdAfx.h"

#include "image_kernel.h"

#include "image_transform.h"

#include <common/exception/exceptions.h>
#include <common/gl/gl_check.h>

#include <Glee.h>

#include <boost/noncopyable.hpp>

#include <unordered_map>

namespace caspar { namespace core {

class shader_program
{
	GLuint program_;
public:

	shader_program() : program_(0) {}
	shader_program(const std::string& vertex_source_str, const std::string& fragment_source_str) : program_(0)
	{
		GLint success;
	
		const char* vertex_source = vertex_source_str.c_str();
						
		auto vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
					
		GL(glShaderSourceARB(vertex_shader, 1, &vertex_source, NULL));
		GL(glCompileShaderARB(vertex_shader));

		GL(glGetObjectParameterivARB(vertex_shader, GL_OBJECT_COMPILE_STATUS_ARB, &success));
		if (success == GL_FALSE)
		{
			char info[2048];
			GL(glGetInfoLogARB(vertex_shader, sizeof(info), 0, info));
			GL(glDeleteObjectARB(vertex_shader));
			std::stringstream str;
			str << "Failed to compile vertex shader:" << std::endl << info << std::endl;
			BOOST_THROW_EXCEPTION(gl::gl_error() << msg_info(str.str()));
		}
			
		const char* fragment_source = fragment_source_str.c_str();
						
		auto fragmemt_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
					
		GL(glShaderSourceARB(fragmemt_shader, 1, &fragment_source, NULL));
		GL(glCompileShaderARB(fragmemt_shader));

		GL(glGetObjectParameterivARB(fragmemt_shader, GL_OBJECT_COMPILE_STATUS_ARB, &success));
		if (success == GL_FALSE)
		{
			char info[2048];
			GL(glGetInfoLogARB(fragmemt_shader, sizeof(info), 0, info));
			GL(glDeleteObjectARB(fragmemt_shader));
			std::stringstream str;
			str << "Failed to compile fragment shader:" << std::endl << info << std::endl;
			BOOST_THROW_EXCEPTION(gl::gl_error() << msg_info(str.str()));
		}
			
		program_ = glCreateProgramObjectARB();
			
		GL(glAttachObjectARB(program_, vertex_shader));
		GL(glAttachObjectARB(program_, fragmemt_shader));

		GL(glLinkProgramARB(program_));
			
		GL(glDeleteObjectARB(vertex_shader));
		GL(glDeleteObjectARB(fragmemt_shader));

		GL(glGetObjectParameterivARB(program_, GL_OBJECT_LINK_STATUS_ARB, &success));
		if (success == GL_FALSE)
		{
			char info[2048];
			GL(glGetInfoLogARB(program_, sizeof(info), 0, info));
			GL(glDeleteObjectARB(program_));
			std::stringstream str;
			str << "Failed to link shader program:" << std::endl << info << std::endl;
			BOOST_THROW_EXCEPTION(gl::gl_error() << msg_info(str.str()));
		}
		GL(glUseProgramObjectARB(program_));
		glUniform1i(glGetUniformLocation(program_, "plane[0]"), 0);
		glUniform1i(glGetUniformLocation(program_, "plane[1]"), 1);
		glUniform1i(glGetUniformLocation(program_, "plane[2]"), 2);
		glUniform1i(glGetUniformLocation(program_, "plane[3]"), 3);
	}

	GLint get_location(const char* name)
	{
		GLint loc = glGetUniformLocation(program_, name);
		return loc;
	}

	shader_program& operator=(shader_program&& other) 
	{
		program_ = other.program_; 
		other.program_ = 0; 
		return *this;
	}

	~shader_program()
	{
		glDeleteProgram(program_);
	}

	void use()
	{	
		GL(glUseProgramObjectARB(program_));		
	}
};

GLubyte progressive_pattern[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xFF, 0xff, 0xff, 0xff, 0xff, 0xff,	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	
GLubyte upper_pattern[] = {
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
		
GLubyte lower_pattern[] = {
	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff,	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff,	0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,	0xff, 0xff, 0xff, 0xff};

struct image_kernel::implementation
{	
	std::unordered_map<pixel_format::type, shader_program> shaders_;

public:
	std::unordered_map<pixel_format::type, shader_program>& shaders()
	{
		GL(glEnable(GL_POLYGON_STIPPLE));
		GL(glEnable(GL_BLEND));
		GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));	

		if(shaders_.empty())
		{
		std::string common_vertex = 
			"void main()															"
			"{																		"
			"	gl_TexCoord[0] = gl_MultiTexCoord0;									"
			"	gl_FrontColor = gl_Color;											"
			"	gl_Position = ftransform();											"
			"}																		";

		std::string common_fragment = 
			"uniform sampler2D	plane[4];											"
			"uniform float		color_gain;											"
																				
			// NOTE: YCbCr, ITU-R, http://www.intersil.com/data/an/an9717.pdf		
			// TODO: Support for more yuv formats might be needed.					
			"vec4 ycbcra_to_bgra(float y, float cb, float cr, float a)				"
			"{																		"
			"	cb -= 0.5;															"
			"	cr -= 0.5;															"
			"	y = 1.164*(y-0.0625);												"
			"																		"
			"	vec4 color;															"
			"	color.r = y + 1.596 * cr;											"
			"	color.g = y - 0.813 * cr - 0.337633 * cb;							"
			"	color.b = y + 2.017 * cb;											"
			"	color.a = a;														"
			"																		"
			"	return color;														"
			"}																		"			
			"																		";
			
		shaders_[pixel_format::abgr] = shader_program(common_vertex, common_fragment +

			"void main()															"
			"{																		"
			"	vec4 abgr = texture2D(plane[0], gl_TexCoord[0].st);					"
			"	gl_FragColor = abgr.argb * color_gain;								"
			"}																		");
		
		shaders_[pixel_format::argb]= shader_program(common_vertex, common_fragment +

			"void main()															"	
			"{																		"
			"	vec4 argb = texture2D(plane[0], gl_TexCoord[0].st);					"
			"	gl_FragColor = argb.grab * gl_Color * color_gain;					"
			"}																		");
		
		shaders_[pixel_format::bgra]= shader_program(common_vertex, common_fragment +

			"void main()															"
			"{																		"
			"	vec4 bgra = texture2D(plane[0], gl_TexCoord[0].st);					"
			"	gl_FragColor = bgra.rgba * gl_Color * color_gain;					"
			"}																		");
		
		shaders_[pixel_format::rgba] = shader_program(common_vertex, common_fragment +

			"void main()															"
			"{																		"
			"	vec4 rgba = texture2D(plane[0], gl_TexCoord[0].st);					"
			"	gl_FragColor = rgba.bgra * gl_Color * color_gain;					"
			"}																		");
		
		shaders_[pixel_format::ycbcr] = shader_program(common_vertex, common_fragment +

			"void main()															"
			"{																		"
			"	float y  = texture2D(plane[0], gl_TexCoord[0].st).r;"
			"	float cb = texture2D(plane[1], gl_TexCoord[0].st).r;"
			"	float cr = texture2D(plane[2], gl_TexCoord[0].st).r;"
			"	float a = 1.0;														"	
			"	gl_FragColor = ycbcra_to_bgra(y, cb, cr, a) * gl_Color * color_gain;"
			"}																		");
		
		shaders_[pixel_format::ycbcra] = shader_program(common_vertex, common_fragment +

			"void main()															"
			"{																		"
			"	float y  = texture2D(plane[0], gl_TexCoord[0].st).r;"
			"	float cb = texture2D(plane[1], gl_TexCoord[0].st).r;"
			"	float cr = texture2D(plane[2], gl_TexCoord[0].st).r;"
			"	float a  = texture2D(plane[3], gl_TexCoord[0].st).r;"
			"	gl_FragColor = ycbcra_to_bgra(y, cb, cr, a) * gl_Color * color_gain;"
			"}																		");
		}
		return shaders_;
	}
};

image_kernel::image_kernel() : impl_(new implementation()){}

void image_kernel::apply(pixel_format::type pix_fmt, const image_transform& transform)
{
	impl_->shaders()[pix_fmt].use();

	GL(glUniform1f(impl_->shaders()[pix_fmt].get_location("color_gain"), static_cast<GLfloat>(transform.get_gain())));

	if(transform.get_mode() == video_mode::upper)
		glPolygonStipple(upper_pattern);
	else if(transform.get_mode() == video_mode::lower)
		glPolygonStipple(lower_pattern);
	else
		glPolygonStipple(progressive_pattern);
}

}}