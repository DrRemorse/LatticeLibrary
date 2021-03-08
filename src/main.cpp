/**
 * Library tests
 * 
**/
extern void test_maths();
extern void test_compression();
extern void test_sockets();
extern void test_opengl_support();
extern void test_opengl_window();
extern void test_opengl_bonerig();

#include <iostream>
#include <cassert>

//using namespace library;

int main(void)
{
  
	return EXIT_SUCCESS;
	
	/////////////////////////////////////////////////////////
	std::cout << "//                 //" << std::endl;
	std::cout << "// Framework tests //" << std::endl;
	std::cout << "//                 //" << std::endl;
	/////////////////////////////////////////////////////////
	
	test_maths();
	return EXIT_FAILURE;
	
	/////////////////////////////////////////////////////////
	std::cout << "****" << std::endl;
	std::cout << "**** TEST: lzo2 compression" << std::endl;
	std::cout << "****" << std::endl;
	std::cout << "" << std::endl;
	/////////////////////////////////////////////////////////
	
	test_compression();
	
	/////////////////////////////////////////////////////////
	std::cout << "****" << std::endl;
	std::cout << "**** TEST: glfw3 window & GL context" << std::endl;
	std::cout << "****" << std::endl;
	std::cout << "" << std::endl;
	/////////////////////////////////////////////////////////
	
	test_opengl_support();
	test_opengl_window();
	
	/////////////////////////////////////////////////////////
	std::cout << "****" << std::endl;
	std::cout << "**** TEST: OpenGL animated bone rig" << std::endl;
	std::cout << "****" << std::endl;
	std::cout << "" << std::endl;
	/////////////////////////////////////////////////////////
	
	test_opengl_bonerig();
	
	/////////////////////////////////////////////////////////
	std::cout << "****" << std::endl;
	std::cout << "**** TEST: Sockets" << std::endl;
	std::cout << "****" << std::endl;
	std::cout << "" << std::endl;
	/////////////////////////////////////////////////////////
	
	test_sockets();
	
	/////////////////////////////////////////////////////////
	std::cout << "//                 //" << std::endl;
	std::cout << "// Tests completed //" << std::endl;
	std::cout << "//                 //" << std::endl;
	/////////////////////////////////////////////////////////
	
	getchar();
	return EXIT_SUCCESS;
}
