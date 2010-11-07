SOURCES=$(wildcard *.cpp)

all: test_pg_driver pg_driver.so

test_pg_driver: test_pg_driver.o
	$(CXX) $^ -ldl -o $@

pg_driver.so: pg_driver.o
	$(CXX) $^ -shared -lpq -o $@

clean:
	rm -f *.d *.o *.so test_pg_driver


-include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	@$(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	    sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	    rm -f $@.$$$$
