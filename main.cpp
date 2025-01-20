/**
 * Minimal program to generate ORM (ambient occlusion, roughness, metallicity) textures
 * as specified in the Khronos documentation: https://www.khronos.org/blog/art-pipeline-for-gltf
 * The channels are:
 * Red: AO
 * Blue: Roughness
 * Green: Metallicity
 *
 * Usage:
 * make
 * ./ormGenerate /mnt/c/ambientcg/materials/DiamondPlate007D_2K-JPG/DiamondPlate007D_2K-JPG_Color.jpg
 *
 * The program assumes the folder contains files with the convention:
 * 
 * - DiamondPlate007D_2K-JPG_Color.jpg - diffuse, not used by this program but assumed as a base path
 * - DiamondPlate007D_2K-JPG_AmbientOcclusion.jpg
 * - DiamondPlate007D_2K-JPG_Roughness.jpg
 * - DiamondPlate007D_2K-JPG_Metalness.jpg
 *
 * If one or several files are missing, they are written as zero in the output ORM.
 * If any file has mismatching size or channel count, the program exits with error.
 * If any of the input files has over 1 channel, only the red channel is read. Generally
 * speaking, these files should be single channel grayscale images.
 * 
 * The output image will be a 3 channel PNG.
 */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <vector>

typedef enum {
	COLOR,
	AO,
	METALLICITY,
	ROUGHNESS
} ImageType;

typedef struct {
	unsigned int w;
	unsigned int h;
	unsigned int channels;
	unsigned char* data;
	std::string fpath;
	ImageType imageType;
} Image;

Image loadImage(const std::string& fpath, ImageType t) {
	Image img;
	int x, y, n;
	img.data = stbi_load(fpath.c_str(), &x, &y, &n, 0);
	img.w = x;
	img.h = y;
	img.channels = n;
	img.fpath = fpath;
	img.imageType = t;

	if (img.data == NULL) {
		std::cout << "Could not read image: " << fpath << ", reason: " << stbi_failure_reason() << std::endl;
	}
	return img;
}

bool replaceStr(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos) {
		return false;
	}
	str.replace(start_pos, from.length(), to);
	return true;
}

bool fileExists(const std::string fpath) {
	struct stat statbuf;
	return (stat(fpath.c_str(), &statbuf) == 0);
}

void cleanup(std::vector<Image>& images) {
	for (auto& iter : images) {
		if (iter.data != NULL) {
			stbi_image_free(iter.data);
			iter.data = NULL;
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Invalid argc. Usage: convertORM path/to/filename.ext format" << std::endl;
		std::cout << "where:" << std::endl;
		std::cout << "file.ext is either a jpg or png file" << std::endl;
		std::cout << "format is the output ORM texture file format, and must be either jpg or png" << std::endl;
		return 1;
	}

	std::string outFormat = argv[2];
	if (outFormat != "jpg" && outFormat != "png") {
		std::cout << "output format must be either jpg or png" << std::endl;
		return 1;
	}

	std::string filepath = argv[1];
	std::string baseName = std::filesystem::path(filepath).filename();
	std::string dirName = std::filesystem::path(filepath).parent_path();

	std::string aoName = baseName;
	std::string rghName = baseName;
	std::string metalName = baseName;
	std::string ormName = baseName;

	replaceStr(aoName, "_Color", "_AmbientOcclusion");
	replaceStr(rghName, "_Color", "_Roughness");
	replaceStr(metalName, "_Color", "_Metalness");
	replaceStr(ormName, "_Color", "_ORM");

	if (outFormat == "jpg") {
		ormName = ormName.substr(0, ormName.find_last_of('.')) + ".jpg";
	} else {
		ormName = ormName.substr(0, ormName.find_last_of('.')) + ".png";
	}

	aoName = dirName + "/" + aoName;
	rghName = dirName + "/" + rghName;
	metalName = dirName + "/" + metalName;
	ormName = dirName + "/" + ormName;

	std::vector<Image> images;

	if (fileExists(aoName)) {
		images.push_back(loadImage(aoName, AO));
	}
	if (fileExists(rghName)) {
		images.push_back(loadImage(rghName, ROUGHNESS));
	}
	if (fileExists(metalName)) {
		images.push_back(loadImage(metalName, METALLICITY));
	}

	if (images.size() == 0) {
		std::cout << "no images" << std::endl;
		return 1;
	}

	unsigned int w = images[0].w;
	unsigned int h = images[0].h;
	unsigned int n = images[0].channels;

	if (n < 1 || n > 3) {
		std::cout << "image channels must be 1, 2, or 3. Got: " << n << std::endl;
		cleanup(images);
		return 1;
	}

	bool ok = true;
	for (unsigned int i = 1; i < images.size(); ++i) {
		if (images[i].w != w) {
			ok = false;
			std::cout << "image " << images[i].fpath << " has wrong width: " << images[i].w << " vs " << w << std::endl;
		}
		if (images[i].h != h) {
			ok = false;
			std::cout << "image " << images[i].fpath << " has wrong height: " << images[i].h << " vs " << h << std::endl;
		}
		if (images[i].channels != n) {
			ok = false;
			std::cout << "image " << images[i].fpath << " has wrong channel count: " << images[i].channels << " vs " << n << std::endl;
		}
	}

	if (!ok) {
		cleanup(images);
		return 1;
	}

	unsigned char* ORM = (unsigned char*)malloc(w*h*3);
	if (!ORM) {
		std::cout << "could not allocate memory" << std::endl;
		cleanup(images);
		return 1;
	}
	memset(ORM, 0, w*h*3);

	for (auto& iter : images) {
		int index = 0;

		switch (iter.imageType) {
			case AO:
				index = 0;
				break;
			case ROUGHNESS:
				index = 1;
				break;
			case METALLICITY:
				index = 2;
				break;
			case COLOR:
				continue;
		}

		unsigned char* dest = ORM;
		for (unsigned y = 0; y < iter.h; y++) {
			// If there are over 1 channels, only read red channel
			for (unsigned x = 0; x < iter.w; x += iter.channels) {
				dest[3 * (y * iter.w + x) + index] = iter.data[iter.channels * (y * iter.w + x)];
			}
		}
	}

	int ret;
	if (outFormat == "png") {
		ret = stbi_write_png(ormName.c_str(), w, h, 3, ORM, w*3);
	} else {
		ret = stbi_write_jpg(ormName.c_str(), w, h, 3, ORM, 99);
	}

	if (ret == 0) {
		std::cout << "failed writing ORM image to " << ormName << ", " << ret << std::endl;
	} else {
		struct stat st;
		stat(ormName.c_str(), &st);
		std::cout << "wrote " << w << "x" << h << " ORM file with size: " << st.st_size / 1024 / 1024 << " MB to: " << ormName << std::endl;
	}

	free(ORM);
	cleanup(images);
	return 0;
}
