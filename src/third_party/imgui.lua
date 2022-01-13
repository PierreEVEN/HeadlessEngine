
target("imgui")
	set_kind("static")
	add_includedirs(".", {public = true})
	for _, ext in ipairs({".h", ".hpp", ".inl", ".natvis"}) do
		add_headerfiles("(**"..ext..")")
	end
	
	for _, ext in ipairs({".c", ".cpp"}) do
		add_files("**.cpp")
	end