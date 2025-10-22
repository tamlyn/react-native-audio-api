#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

#include <audioapi/ios/core/IOSAudioFileOptions.h>

namespace audioapi {

IOSAudioFileOptions::IOSAudioFileOptions(
  float sampleRate,
  size_t channelCount,
  size_t bitRate,
  size_t flags)
{
  sampleRate_ = sampleRate;
  channelCount_ = channelCount;
  bitRate_ = bitRate;

  format_ = static_cast<uint8_t>(((flags >> 0) & 0xF));
  quality_ = static_cast<uint8_t>(((flags >> 4) & 0xF));
  flacCompressionLevel_ = static_cast<uint8_t>(((flags >> 8) & 0xF));
  directory_ = static_cast<uint8_t>(((flags >> 12) & 0xF));
  bitDepth_ = static_cast<uint8_t>(((flags >> 16) & 0xF));
}

AudioFormatID IOSAudioFileOptions::getFormat() const {
  switch (format_) {
    case 1: return kAudioFormatLinearPCM;  // WAV <-> Linear PCM container
    case 2: return kAudioFormatLinearPCM;  // CAF <-> Linear PCM container
    case 3: return kAudioFormatMPEG4AAC;   // M4A <-> AAC
    case 4: return kAudioFormatFLAC;       // FLAC <-> FLAC
    default: return kAudioFormatLinearPCM; // Default to Linear PCM
  }
}

NSInteger IOSAudioFileOptions::getQuality() const {
  switch (quality_) {
    case 1: return AVAudioQualityMin;
    case 2: return AVAudioQualityLow;
    case 3: return AVAudioQualityMedium;
    case 4: return AVAudioQualityHigh;
    case 5: return AVAudioQualityMax;
    default: return AVAudioQualityMedium; // Default to Medium
  }
}


NSInteger IOSAudioFileOptions::getFlacCompressionLevel() const {
  // Shift the quality from 1-9 to 0-8 for real values
  // Default to 5 if out of range
  return (flacCompressionLevel_ >= 1 && flacCompressionLevel_ <= 9)
    ? flacCompressionLevel_ - 1
    : 5;
}

NSSearchPathDirectory IOSAudioFileOptions::getDirectory() const {
  switch (directory_) {
    case 1: return NSDocumentDirectory;
    case 2: return NSCachesDirectory;
    default: return NSCachesDirectory; // Default to Caches
  }
}

NSString *IOSAudioFileOptions::getFileExtension() const {
  switch (format_) {
    case 1: return @"wav";  // WAV <-> Linear PCM container
    case 2: return @"caf";  // CAF <-> Linear PCM container
    case 3: return @"m4a";  // M4A <-> AAC
    case 4: return @"flac"; // FLAC <-> FLAC
    default: return @"wav"; // Default to WAV
  }
}

NSUInteger IOSAudioFileOptions::getBitDepth() const {
  switch (bitDepth_) {
    case 1: return 16;
    case 2: return 24;
    case 3: return 32;
    default: return 24; // Default to 24-bit
  }
}

NSDictionary *IOSAudioFileOptions::getFileSettings() {
  AudioFormatID format = getFormat();
  NSMutableDictionary *settings = [NSMutableDictionary dictionary];

  settings[AVFormatIDKey] = @(format);
  settings[AVSampleRateKey] = @(sampleRate_);
  settings[AVNumberOfChannelsKey] = @(channelCount_);

  settings[AVEncoderAudioQualityKey] = @(getQuality());

  if (format == kAudioFormatMPEG4AAC) {
    settings[AVEncoderBitRateKey] = @(bitRate_);
  }

  if (format == kAudioFormatLinearPCM) {
    NSInteger bitDepth = getBitDepth();

    settings[AVLinearPCMBitDepthKey] = @(bitDepth);
    settings[AVLinearPCMIsFloatKey] = @(bitDepth == 32);
    settings[AVLinearPCMIsBigEndianKey] = @(NO);
    settings[AVLinearPCMIsNonInterleaved] = @(NO);
  }

  if (format == kAudioFormatFLAC) {
    settings[@"FLACCompressionLevel"] = @(getFlacCompressionLevel());
  }

  return settings;
}

} // namespace audioapi
