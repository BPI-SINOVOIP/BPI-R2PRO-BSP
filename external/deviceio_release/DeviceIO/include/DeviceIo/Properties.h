/*
 * Copyright (c) 2014 Fredy Wijaya
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DEVICEIO_FRAMEWORK_PROPERTIES_H_
#define DEVICEIO_FRAMEWORK_PROPERTIES_H_

#include <string>
#include <vector>
#include <map>

namespace DeviceIOFramework {

class Properties {
public:
	/**
	 * Get single instance of Properties
	 */
	static Properties* getInstance();

	/**
	 * Read properties to map
	 */
	int init();

	/**
	 * Gets the property value from a given key.
	 *
	 * This method throws a PropertyNotFoundException when a given key does not
	 * exist.
	 */
	std::string get(const std::string& key) const;

	/**
	 * Gets the property value from a given key. Use a default value if not found.
	 */
	std::string get(const std::string& key, const std::string& defaultValue) const;

	/**
	 * Gets the list of property names.
	 */
	std::vector<std::string> getPropertyNames() const;

	/**
	 * Adds a new property. If the property already exists, it'll overwrite
	 * the old one.
	 */
	void set(const std::string& key, const std::string& value);

	/**
	 * Removes the property from a given key.
	 *
	 * If the property doesn't exist a PropertyNotFoundException will be thrown.
	 */
	void remove(const std::string& key);

	virtual ~Properties();
private:
	Properties();
	Properties(const Properties&){};
	Properties& operator=(const Properties&){return *this;};

	/* Properties single instance */
	static Properties* m_instance;
	/* Properties keys */
	std::vector<std::string> keys;
	/* Map of properties */
	std::map<std::string, std::string> properties;
};
} // namespace framework

#endif /* DEVICEIO_FRAMEWORK_PROPERTIES_H_ */
