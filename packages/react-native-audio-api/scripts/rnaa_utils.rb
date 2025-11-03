def check_if_worklets_enabled(app_node_modules_path)
  validate_worklets_script = File.expand_path(File.join(__dir__, 'validate-worklets-version.js'))

  # Set NODE_PATH so Node.js can resolve react-native-worklets from the app's node_modules
  env = { 'NODE_PATH' => app_node_modules_path }

  unless system(env, "node \"#{validate_worklets_script}\"")
    # If the validation script fails, we assume worklets are not present or have an incompatible version
    puts "Worklets are not enabled or have an incompatible version. Building without worklets support."
    return false
  end
  puts "Worklets are enabled and have a compatible version."
  true
end
