Import("env")

# Prepend the project's tools directory to PATH so our local wrappers take precedence
import os
project_dir = env.subst("${PROJECT_DIR}")
tools_dir = os.path.join(project_dir, "tools")

env.AppendENVPath("PATH", tools_dir)

