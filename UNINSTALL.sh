cd build
cat install_manifest.txt | sudo xargs rm
cat install_manifest.txt | xargs -L1 dirname | sudo xargs rmdir -p
