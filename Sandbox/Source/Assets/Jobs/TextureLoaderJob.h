#pragma once

#include "Job.h"

class TextureLoaderJob
	: public Job
{
public:
	void DoWork() override;
};