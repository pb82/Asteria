all:
	@protoc Event.proto --cpp_out .

install:
	@mv Event.pb.cc ../Serialization/
	@mv Event.pb.h  ../Serialization/

clean:
	@rm -f ./Event.pb.cc
	@rm -f ./Event.pb.h
