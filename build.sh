BUILD_DIR="$(pwd)/build"
BUILD_TYPE="$LOXY_BUILD_TYPE"

if [[ -z "$BUILD_TYPE" ]]; then
  BUILD_TYPE="Debug"
fi

function gen_makefiles {
  if [[ ! -d "$BUILD_DIR" ]]; then
    mkdir -p $BUILD_DIR
  fi

  cd $BUILD_DIR
  echo $(pwd)
  cmake -D CMAKE_BUILD_TYPE="$BUILD_TYPE" ..
}

function build {
  gen_makefiles
  make
}

build