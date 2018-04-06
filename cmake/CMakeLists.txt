#     Copyright (C) 2014-2018 Sumandeep Banerjee
# 
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU Lesser General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU Lesser General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.

# File:   
# Author: sumandeep
# Email:  sumandeep.banerjee@gmail.com

cmake_minimum_required(VERSION 2.8)
project( ImageSearch )
find_package( OpenCV REQUIRED )

# test project
add_executable( ImageSearch_test source/test_main.cpp source/Common.cpp source/SearchEngine.cpp source/ImageDB.cpp source/VocabTree.cpp source/ImageHash.cpp )
target_link_libraries( ImageSearch_test ${OpenCV_LIBS} )

# server project
add_executable( ImageSearch_server source/server_main.cpp source/Common.cpp source/SearchEngine.cpp source/ImageDB.cpp source/VocabTree.cpp source/ImageHash.cpp )
target_link_libraries( ImageSearch_server ${OpenCV_LIBS} )

# client project
add_executable( ImageSearch_client source/client_main.cpp )
target_link_libraries( ImageSearch_client )

