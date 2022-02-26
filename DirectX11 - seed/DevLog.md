# Dev Log

2/17: 
	* Start wraps initialization of factory, debugController, apapter, device, debugDevice
	* Might need to take a look at clang-format and how to develop project on vscode
2/18:
	* Wrap initialization of command queue, command allocator, fence, root signature
	* Close Clang-format for now
2/20:
	* Wrap initialization of command list, vertex buffer, index buffer, uniform buffer object, pipeline state object (PSO), shader program
	* Wrap wait for fence
	* Become harder and harder to wrap some initializations
	* [TODO]
		* Do we need to modify PSO when we want to draw something transparent? (e.g. use blend only for draw calls of transparent)?
		* Or eventually we will need PSO for each draw call?