#pragma once
#include "FileBase.h"
class ImageFile : public FileBase
{
public:
	ImageFile(FileManager& manager);
	~ImageFile();

private:
	void Delete() override;
};

