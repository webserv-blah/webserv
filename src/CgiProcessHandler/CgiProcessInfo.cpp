#include "CgiProcessInfo.hpp"

CgiProcessInfo::CgiProcessInfo()
	: pid_(-1), finished_(false) {
	inPipe_[0] = -1;
	inPipe_[1] = -1;
	outPipe_[0] = -1;
	outPipe_[1] = -1;
}

void CgiProcessInfo::initializePipes() {
	if (pipe(inPipe_) == -1) {
		throw std::runtime_error("Failed to create input pipe.");
	}

	if (pipe(outPipe_) == -1) {
		// 첫 번째 pipe()가 성공했지만 두 번째가 실패하면, 리소스 해제
		close(inPipe_[0]);
		close(inPipe_[1]);
		throw std::runtime_error("Failed to create output pipe.");
	}
}

void CgiProcessInfo::closePipes() {
	if (inPipe_[0] != -1) {
		close(inPipe_[0]);
		inPipe_[0] = -1;
	}

	if (inPipe_[1] != -1) {
		close(inPipe_[1]);
		inPipe_[1] = -1;
	}

	if (outPipe_[0] != -1) {
		close(outPipe_[0]);
		outPipe_[0] = -1;
	}

	if (outPipe_[1] != -1) {
		close(outPipe_[1]);
		outPipe_[1] = -1;
	}
}
