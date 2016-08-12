import swig.direct.CPLDirect.*;

import java.util.Vector;


public class CPLPropertyEntry {

	/// The provenance object and a version
    private CPLAncestryEntry ancestryEntry;

	/// The property name
	private String key;

	/// The property value
	private String value;


	CPLAncestryPropertyEntry(CPLAncestryEntry ancestryEntry, String key, String value) {
        this.ancestryEntry = ancestryEntry;
        this.key = key;
        this.value = value;
	}


	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLPropertyEntry) {
			CPLPropertyEntry o = (CPLPropertyEntry) other;
			return o.ancestryEntry.equals(ancestryEntry)
                && o.key.equals(key)
                && (o.value == value ? true : o.value.equals(value));
		}
		else {
			return false;
		}
	}


	/**
	 * Compute the hash code of this object
	 * TODO double check this isnt dumb
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
        int valueHashCode = value == null ? 0 : value.hashCode();
		return ((ancestryEntry.hashCode() * 31) << 4)
            ^ ((key.hashCode() * 31) ^ ~valueHashCode);
	}


	/**
	 * Return a string representation of the object. Note that this is based
	 * on the internal object ID, since the name might not be known.
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		return ancestryEntry.toString() + "-" + key + " = " + value;
	}

	public CPLAncestryEntry getAncestryEntry() {
		return ancestryEntry;
	}

	/**
	 * Get the property name (key)
	 *
	 * @return the key
	 */
	public String getKey() {
		return key;
	}


	/**
	 * Get the property value
	 *
	 * @return the value
	 */
	public String getValue() {
		return value;
	}


	/**
	 * Create a more detailed string representation of the object
	 *
	 * @param includeObject whether to also include the object and version
	 * @return a string describing the entry
	 */
	public String toString(boolean includeObject) {

		return (includeObject ? ancestryEntry.toString() + "-" : "")
            + key + " = " + value;
	}
}
