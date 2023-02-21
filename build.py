import os
# from glob import iglob
from subprocess import call

libs_path = '.\\module\\libs'

os.system('cd .\\module && ndk-build.cmd ')

for subdir, dirs, files in os.walk(libs_path):
    for d in dirs:
        file_name = d + '.so'
        for subdirs, dirs, file in os.walk(libs_path + '/' + d):
            os.system('copy '+ ' /Y .\\module\\libs\\'+ d +'\\libzzzzzzxample.so .\\zygisk-halt-module\\zygisk\\' +  file_name )


os.system(' cd .\\zygisk-halt-module && zip -r zygisk-halt-module.zip ./*' )

os.system('adb push ./zygisk-halt-module/zygisk-halt-module.zip /data/local/tmp' )

os.system('adb shell cp /data/local/tmp/zygisk-halt-module.zip /storage/self/primary/Download/' )


       


