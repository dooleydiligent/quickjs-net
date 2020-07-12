export class Dictionary {
  constructor() {
    this.items = {}
  }
  /**
   * Remove all elements - reset to empty
   */
  clear() {
    Object.keys(this.items).forEach((key) => {
      delete this.items[key];
    });
  }
  /**
   * Check if an item exists - by value
   * @param value The item required
   * @returns
   */
  contains(value) {
    return this.values().indexOf(value) !== -1;
  }
  /**
   * Check if a key exists
   * @param key The key required
   * @returns
   */
  exists(key) {
    return !!this.items[key];
  }
  /**
   * Get an item by key
   * @param key The element key
   * @returns
   */
  get(key) {
    return this.items[key];
  }
  /**
   * The array of keys in the store
   * @returns
   */
  keys() {
    return Object.keys(this.items);
  }
  /**
   * Put a new key/value pair
   * Overwrites an existing key
   * @param key The key
   * @param value The value - supports null
   * @returns
   */
  put(key, value) {
    this.items[key] = value;
    return value;
  }
  /**
   * Remove an item by key
   * @param key The key to remove
   * @returns
   */
  remove(key) {
    const item = this.items[key];
    if (item) {
      delete this.items[key];
    }
    return item;
  }
  /**
   * Report the number of keys in the store
   * @returns
   */
  size() {
    return Object.keys(this.items).length;
  }
  /**
   * The array of values
   * @returns
   */
  values() {
    return Object.values(this.items);
  }
}
