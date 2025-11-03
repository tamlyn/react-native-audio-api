require "json"
require_relative './scripts/rnaa_utils'

package_json = JSON.parse(File.read(File.join(__dir__, "package.json")))

$new_arch_enabled = ENV['RCT_NEW_ARCH_ENABLED'] == '1'

folly_flags = "-DFOLLY_MOBILE=1 -DFOLLY_USE_LIBCPP=1 -Wno-comma -Wno-shorten-64-to-32"
fabric_flags = $new_arch_enabled ? '-DRCT_NEW_ARCH_ENABLED' : ''
version_flag = "-DAUDIOAPI_VERSION=#{package_json['version']}"

# Determine the app's node_modules path
# Try to get it from Pod::Config if available, otherwise use the podspec directory
# Note: This should be done here because check_if_worklets_enabled cant access Pod::Config and __dir__ is also not reliable
begin
  # Pod::Config.instance.project_pods_root gives us the ios/Pods directory
  # Go up one level to ios/, then up again to the app root
  ios_dir = File.join(Pod::Config.instance.project_pods_root, '..')
  app_root = File.expand_path('..', ios_dir)
  app_node_modules = File.join(app_root, 'node_modules')
rescue
  # Fallback if Pod::Config is not available yet (shouldn't happen but just in case)
  parent_dir = File.dirname(File.expand_path(__dir__))
  app_node_modules = File.basename(parent_dir) == 'node_modules' ? parent_dir : File.join(parent_dir, 'node_modules')
end

worklets_preprocessor_flag = check_if_worklets_enabled(app_node_modules) ? '-DRN_AUDIO_API_ENABLE_WORKLETS=1' : ''

Pod::Spec.new do |s|
  s.name         = "RNAudioAPI"
  s.version      = package_json["version"]
  s.summary      = package_json["description"]
  s.homepage     = package_json["homepage"]
  s.license      = package_json["license"]
  s.authors      = package_json["author"]

  s.platforms    = { :ios => min_ios_version_supported }
  s.source       = { :git => "https://github.com/software-mansion/react-native-audio-api.git", :tag => "#{s.version}" }

  s.subspec "audioapi" do |ss|
    ss.source_files = "common/cpp/audioapi/**/*.{cpp,c,h,hpp}"
    ss.header_dir = "audioapi"
    ss.header_mappings_dir = "common/cpp/audioapi"

    ss.subspec "ios" do |sss|
      sss.source_files = "ios/audioapi/**/*.{mm,h,m,hpp}"
      sss.header_dir = "audioapi"
      sss.header_mappings_dir = "ios/audioapi"
    end

    ss.subspec "audioapi_dsp" do |sss|
      sss.source_files = "common/cpp/audioapi/dsp/**/*.{cpp}"
      sss.header_dir = "audioapi/dsp"
      sss.header_mappings_dir = "common/cpp/audioapi/dsp"
      sss.compiler_flags = "-O3"
    end
  end

  s.ios.frameworks = 'CoreFoundation', 'CoreAudio', 'AudioToolbox', 'Accelerate', 'MediaPlayer', 'AVFoundation'

  s.compiler_flags = "#{folly_flags}"

  s.prepare_command = <<-CMD
    chmod +x scripts/download-prebuilt-binaries.sh
    scripts/download-prebuilt-binaries.sh ios
  CMD

  # Assumes Pods dir is nested under ios project dir
  ios_dir = File.join(Pod::Config.instance.project_pods_root, '..')
  rn_audio_dir_relative = Pathname.new(__dir__).relative_path_from(ios_dir).to_s

  external_dir_relative = "common/cpp/audioapi/external"
  lib_dir = "$(PROJECT_DIR)/#{rn_audio_dir_relative}/#{external_dir_relative}/$(PLATFORM_NAME)"

  s.ios.vendored_frameworks = [
    'common/cpp/audioapi/external/libavcodec.xcframework',
    'common/cpp/audioapi/external/libavformat.xcframework',
    'common/cpp/audioapi/external/libavutil.xcframework',
    'common/cpp/audioapi/external/libswresample.xcframework'
  ]
s.pod_target_xcconfig = {
  "USE_HEADERMAP" => "YES",
  "CLANG_CXX_LANGUAGE_STANDARD" => "c++20",
  "GCC_PREPROCESSOR_DEFINITIONS" => '$(inherited) HAVE_ACCELERATE=1',
  "HEADER_SEARCH_PATHS" => %W[
    $(PODS_TARGET_SRCROOT)/common/cpp
    $(PODS_TARGET_SRCROOT)/ios
    $(PODS_ROOT)/Headers/Public/RNWorklets
    $(PODS_ROOT)/Headers/Private/React-Core
    $(PODS_TARGET_SRCROOT)/#{external_dir_relative}/include
    $(PODS_TARGET_SRCROOT)/#{external_dir_relative}/include/opus
    $(PODS_TARGET_SRCROOT)/#{external_dir_relative}/include/vorbis
    $(PODS_TARGET_SRCROOT)/#{external_dir_relative}/ffmpeg_include
  ].join(" "),
  'OTHER_CFLAGS' => "$(inherited) #{folly_flags} #{fabric_flags} #{version_flag} #{worklets_preprocessor_flag}",
  'OTHER_CPLUSPLUSFLAGS' => "$(inherited) #{folly_flags} #{fabric_flags} #{version_flag} #{worklets_preprocessor_flag}",
}

s.user_target_xcconfig = {
  'OTHER_LDFLAGS' => %W[
    $(inherited)
    -force_load #{lib_dir}/libopusfile.a
    -force_load #{lib_dir}/libopus.a
    -force_load #{lib_dir}/libogg.a
    -force_load #{lib_dir}/libvorbis.a
    -force_load #{lib_dir}/libvorbisenc.a
    -force_load #{lib_dir}/libvorbisfile.a
    -force_load #{lib_dir}/libssl.a
    -force_load #{lib_dir}/libcrypto.a
  ].join(" "),
  'HEADER_SEARCH_PATHS' => %W[
    $(inherited)
    $(PODS_ROOT)/Headers/Public/RNAudioAPI
    $(PODS_TARGET_SRCROOT)/common/cpp
  ].join(' ')
}
  # Use install_modules_dependencies helper to install the dependencies if React Native version >=0.71.0.
  # See https://github.com/facebook/react-native/blob/febf6b7f33fdb4904669f99d795eba4c0f95d7bf/scripts/cocoapods/new_architecture.rb#L79.
  install_modules_dependencies(s)
end
