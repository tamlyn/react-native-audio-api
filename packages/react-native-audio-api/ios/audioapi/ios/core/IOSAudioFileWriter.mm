#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/IOSAudioFileWriter.h>

namespace audioapi {

IOSAudioFileWriter::IOSAudioFileWriter() {}

IOSAudioFileWriter::~IOSAudioFileWriter()
{
  // [self closeCurrentFile];
}

void IOSAudioFileWriter::openFile() {}

void IOSAudioFileWriter::closeFile() {}

bool IOSAudioFileWriter::writeAudioData(const AudioBufferList *audioBufferList, int numFrames) {}

} // namespace audioapi
