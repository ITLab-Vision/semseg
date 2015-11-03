#include <iostream>
#include <fstream>
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\video\video.hpp"

static const char argsDefs[] = 
	"{ | list             |     | Path to target images list file }"
	"{ | frames_per_image | 1   | Subj.                           }"
    "{ | video            |     | Resulting video name            }"
	"{ | width            | 400 | Output video frame width        }"
	"{ | height           | 300 | Output video frame height       }";

void printHelp(std::ostream& os) {
	os << "\tUsage: --list=path/to/list.txt --video=output/filename" << std::endl
	   << "\t[--frames_per_image=<number>] [--width=400] [--height=300]" << std::endl;
}

namespace ReturnCode {
	enum {
		Success = 0,
		InputFileNotFound = 1,
		OutputFileNotSpecefied = 2
	};
}; //namespace ReturnCode

class MovieMaker {
public:
	MovieMaker(int frameWidth, int frameHeight, int frameRepeat = 1) :
		frameWidth(frameWidth), frameHeight(frameHeight), frameRepeat(frameRepeat)
	{}

	void createVideo(std::istream& list, const std::string& outputFile);
private:
	int frameWidth;
	int frameHeight;
	int frameRepeat;

	void preprocessImage(cv::Mat& image);
};

void MovieMaker::createVideo(std::istream& list, const std::string& outputFileName) {
	cv::VideoWriter videoWriter;

	std::string tempFile = cv::tempfile(outputFileName.substr(outputFileName.rfind('.')).c_str());
	std::cout << "Opening file '" << tempFile << "' for writing." << std::endl; 	
	if (videoWriter.open(outputFileName, CV_FOURCC('X', 'V', 'I', 'D'), 30.0, cv::Size(frameWidth, frameHeight)) == false) {
		throw std::exception("Failed to open file for video writing.");
	}

	while (list.good() == true) {
		std::string fileName;
		std::getline(list, fileName);

		cv::Mat image = cv::imread(fileName);
		preprocessImage(image);

		for(int i = 0; i < frameRepeat; ++i) {
			videoWriter << image;
		}
	}
	videoWriter.release();
}

void MovieMaker::preprocessImage(cv::Mat& image) {
	// Crop image
	if ((image.rows != frameHeight) || (image.cols != frameWidth)) {
		cv::Mat resized = cv::Mat::zeros(frameWidth, frameHeight, image.type());
		image(cv::Range(0, std::min(image.rows, frameWidth)), 
			  cv::Range(0, std::min(image.cols, frameHeight))
			 ).copyTo(resized);
		image = resized;
	}
}



int main(int argc, char* argv[]) {
	cv::CommandLineParser parser(argc, argv, argsDefs);
	auto listFileName = parser.get<std::string>("list");
	auto videoFileName = parser.get<std::string>("video");

	// Check command line args
	if (listFileName.empty() == true) {
		std::cerr << "Input file is not specefied." << std::endl;
		printHelp(std::cerr);
		return ReturnCode::InputFileNotFound;
	}
	if (videoFileName.empty() == true) {
		std::cerr << "Output file is not specefied." << std::endl;
		printHelp(std::cerr);
		return ReturnCode::OutputFileNotSpecefied;
	}
	int framesPerImage = parser.get<int>("frames_per_image");
	if (framesPerImage < 1) {
		framesPerImage = 1;
	}
	int frameWidth = parser.get<int>("width");
	int frameHeight = parser.get<int>("height");
	if (frameWidth < 0) {
		frameWidth = 400; //magic
	}
	if (frameHeight < 0) {
		frameHeight = 300; //magic
	}

	std::ifstream list(listFileName);
	if (list.is_open() == false) {
		std::cerr << "File '" << listFileName << "' not found. Exiting." << std::endl;
		return ReturnCode::InputFileNotFound;
	}
	
	MovieMaker maker(frameWidth, frameHeight, framesPerImage);
	try {
		maker.createVideo(list, videoFileName);
		std::cout << "Video successfully created at: '" << videoFileName << "'." << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

