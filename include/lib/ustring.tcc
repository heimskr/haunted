template <typename... Args>
size_t find(Args && ...args) const {
	return std::string(*this).find(std::forward<Args>(args)...);
}

template <typename... Args>
size_t find_first_of(Args && ...args) const {
	return std::string(*this).find_first_of(std::forward<Args>(args)...);
}

template <typename... Args>
size_t find_first_not_of(Args && ...args) const {
	return std::string(*this).find_first_not_of(std::forward<Args>(args)...);
}

template <typename... Args>
size_t find_last_of(Args && ...args) const {
	return std::string(*this).find_last_of(std::forward<Args>(args)...);
}

template <typename... Args>
size_t find_last_not_of(Args && ...args) const {
	return std::string(*this).find_last_not_of(std::forward<Args>(args)...);
}
