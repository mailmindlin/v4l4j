package au.edu.jcu.v4l4j.impl.json;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Implementation of the <a href="https://tools.ietf.org/id/draft-snell-json-test-01.html">JSON predicate (Snell) spec</a>,
 * with an extension to support the <strong>contained</strong> and <strong>intersects</strong> ops.
 * @author mailmindlin
 */
public interface JSONPredicate {
	
	public static JSONPredicate compile(JSONObject raw) {
		switch (raw.getString("op")) {
			case "contains":
				return new JSONContainsPredicate(raw.getString("path"), raw.getString("value"), raw.optBoolean("ignore_case", false));
			case "defined":
				return new JSONDefinedPredicate(raw.getString("path"));
			case "ends":
				return new JSONEndsPredicate(raw.getString("path"), raw.getString("value"), raw.optBoolean("ignore_case", false));
			case "in":
			case "less":
				return new JSONLessPredicate(raw.getString("path"), raw.getDouble("value"));
			case "matches":
			case "more":
				return new JSONMorePredicate(raw.getString("path"), raw.getDouble("value"));
			case "starts":
				return new JSONStartsPredicate(raw.getString("path"), raw.getString("value"), raw.optBoolean("ignore_case", false));
			case "test":
				if (raw.optString("value") != null)
					return new JSONStringTestPredicate(raw.getString("path"), raw.getString("value"), raw.optBoolean("ignore_case", false));
				//TODO finish
				throw new UnsupportedOperationException();
			case "type":
				return new JSONTypePredicate(raw.getString("path"), raw.getString("value"));
			case "undefined":
				return new JSONUndefinedPredicate(raw.getString("path"));
			case "contained":
			case "intersects":
			case "and": {
				ArrayList<JSONPredicate> children = new ArrayList<>();
				JSONArray childDefs = raw.getJSONArray("apply");
				for (int i = 0, l = childDefs.length(); i < l; i++)
					children.add(compile(childDefs.getJSONObject(i)));
				return new JSONAndPredicate(children);
			}
			case "not": {
				ArrayList<JSONPredicate> children = new ArrayList<>();
				JSONArray childDefs = raw.getJSONArray("apply");
				for (int i = 0, l = childDefs.length(); i < l; i++)
					children.add(compile(childDefs.getJSONObject(i)));
				return new JSONNotPredicate(children);
			}
			case "or": {
				ArrayList<JSONPredicate> children = new ArrayList<>();
				JSONArray childDefs = raw.getJSONArray("apply");
				for (int i = 0, l = childDefs.length(); i < l; i++)
					children.add(compile(childDefs.getJSONObject(i)));
				return new JSONOrPredicate(children);
			}
			default:
				throw new IllegalArgumentException("Undefined operation: " + raw.getString("op"));
		}
	}
	
	public static boolean matches(JSONObject predicate, JSONObject target) {
		//TODO can we do this without compiling?
		return compile(predicate).test(target);
	}
	
	public static Optional<Object> traverse(JSONObject object, int offset, String...path) {
		if (object == null)
			return Optional.empty();
		String segment = path[offset];
		if (!object.has(segment))
			return Optional.empty();
		
		if (offset >= path.length - 1)
			return Optional.of(object.get(segment));
		
		{
			JSONObject nvo = object.optJSONObject(segment);
			if (nvo != null) {
				if (offset == path.length - 1)
					return Optional.of(nvo);
				return traverse(nvo, offset + 1, path);
			}
		}
		{
			JSONArray nva = object.optJSONArray(segment);
			if (nva != null) {
				if (offset == path.length - 1)
					return Optional.of(nva);
				return traverse(nva, offset + 1, path);
			}
		}
		//TODO finish
		return Optional.empty();
	}
	
	public static Optional<Object> traverse(JSONArray array, int offset, String...path) {
		if (array == null)
			return Optional.empty();
		
		int idx;
		try {
			idx = Integer.parseInt(path[offset]);
		} catch (NumberFormatException e) {
			return Optional.empty();
		}
		if (idx < 0 || idx >= array.length())
			return Optional.empty();
		
		if (offset >= path.length - 1)
			return Optional.of(array.get(idx));
		
		{
			JSONObject nvo = array.optJSONObject(idx);
			if (nvo != null) {
				if (offset == path.length - 1)
					return Optional.of(nvo);
				return traverse(nvo, offset + 1, path);
			}
		}
		{
			JSONArray nva = array.optJSONArray(idx);
			if (nva != null) {
				if (offset == path.length - 1)
					return Optional.of(nva);
				return traverse(nva, offset + 1, path);
			}
		}
		//TODO finish
		return Optional.empty();
	}
	
	public static Optional<Object> traverse(Map<String, Object> map, int offset, String...path) {
		if (map == null)
			return Optional.empty();
		//TODO finish
		return Optional.empty();
	}
	
	public static Optional<Object> traverse(List<Object> list, int offset, String...path) {
		if (list == null)
			return Optional.empty();
		
		int idx;
		try {
			idx = Integer.parseInt(path[offset]);
		} catch (NumberFormatException e) {
			return Optional.empty();
		}
		if (idx < 0 || idx >= list.size())
			return Optional.empty();
		
		Object v = list.get(idx);
		if (offset >= path.length - 1) {
			return Optional.of(v);
		} else if (v instanceof JSONObject) {
			return traverse((JSONObject)v, offset + 1, path);
		} else if (v instanceof JSONArray) {
			return traverse((JSONArray)v, offset + 1, path);
		} else if (v instanceof Map) {
			@SuppressWarnings("unchecked")
			Map<String, Object> m = (Map<String, Object>) v;
			return traverse(m, offset + 1, path);
		} else if (v instanceof List) {
			@SuppressWarnings("unchecked")
			List<Object> l = (List<Object>) v;
			return traverse(l, offset + 1, path);
		}
		
		//TODO finish
		return Optional.empty();
	}
	
	boolean test(JSONObject object);
	boolean test(JSONArray array);
	boolean test(Map<String, Object> object);
	boolean test(List<Object> array);
	
	public static abstract class AbstractJSONValuePredicate implements JSONPredicate {
		protected final String[] path;
		
		public AbstractJSONValuePredicate(String[] path) {
			this.path = path;
		}
		
		@Override
		public boolean test(JSONObject object) {
			return test(traverse(object, 0, path));
		}
		
		@Override
		public boolean test(JSONArray array) {
			return test(traverse(array, 0, path));
		}
		
		@Override
		public boolean test(Map<String, Object> object) {
			return test(traverse(object, 0, path));
		}
		
		@Override
		public boolean test(List<Object> array) {
			return test(traverse(array, 0, path));
		}
		
		protected abstract boolean test(Optional<Object> value);
	}
	
	public static class JSONContainsPredicate extends AbstractJSONValuePredicate {
		protected final String value;
		protected final boolean ignoreCase;
		
		public JSONContainsPredicate(String path, String value, boolean ignoreCase) {
			super(path.split("/"));
			this.ignoreCase = ignoreCase;
			if (ignoreCase)
				this.value = value.toLowerCase();
			else
				this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			String str = "" + value.get();
			if (ignoreCase)
				str = str.toLowerCase();
			return str.contains(this.value);
		}
	}
	
	public static class JSONDefinedPredicate extends AbstractJSONValuePredicate {
		public JSONDefinedPredicate(String path) {
			super(path.split("/"));
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			return value.isPresent();
		}
	}
	
	public static class JSONEndsPredicate extends AbstractJSONValuePredicate {
		protected final String value;
		protected final boolean ignoreCase;
		
		public JSONEndsPredicate(String path, String value, boolean ignoreCase) {
			super(path.split("/"));
			this.ignoreCase = ignoreCase;
			if (this.ignoreCase)
				this.value = value.toLowerCase();
			else
				this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			String str = "" + value.get();
			if (ignoreCase)
				str = str.toLowerCase();
			return str.endsWith(this.value);
		}
	}
	
	public static class JSONLessPredicate extends AbstractJSONValuePredicate {
		protected final Number value;
		
		public JSONLessPredicate(String path, Number value) {
			super(path.split("/"));
			this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			Object o = value.get();
			Number n = null;
			if (o instanceof Number)
				n = (Number) o;
			//TODO parse string=>number
			if (n == null)
				return false;
			return n.doubleValue() < this.value.doubleValue();
		}
	}
	
	public static class JSONMorePredicate extends AbstractJSONValuePredicate {
		protected final Number value;
		
		public JSONMorePredicate(String path, Number value) {
			super(path.split("/"));
			this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			Object o = value.get();
			Number n = null;
			if (o instanceof Number)
				n = (Number) o;
			//TODO parse string=>number
			if (n == null)
				return false;
			return n.doubleValue() > this.value.doubleValue();
		}
	}
	
	public static class JSONStartsPredicate extends AbstractJSONValuePredicate {
		protected final String value;
		protected final boolean ignoreCase;
		
		public JSONStartsPredicate(String path, String value, boolean ignoreCase) {
			super(path.split("/"));
			this.ignoreCase = ignoreCase;
			if (this.ignoreCase)
				this.value = value.toLowerCase();
			else
				this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			String str = "" + value.get();
			if (ignoreCase)
				str = str.toLowerCase();
			return str.startsWith(this.value);
		}
	}
	
	public static class JSONTypePredicate extends AbstractJSONValuePredicate {
		protected final String value;
		
		public JSONTypePredicate(String path, String value) {
			super(path.split("/"));
			this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> maybe) {
			String type = null;
			if (!maybe.isPresent()) {
				type = "undefined";
			} else {
				Object value = maybe.get();
				if (value == null)
					type = "null";
				else if (value instanceof String)
					type = "string";
				else if (value instanceof Number)
					type = "number";
				else if (value instanceof Boolean)
					type = "boolean";
				else if (value instanceof JSONObject || value instanceof Map)
					type = "object";
				else if (value instanceof JSONArray || value instanceof List)
					value = "array";
				//TODO support 'date', 'date-time', 'time', 'lang', 'lang-range', 'iri', 'absolute-iri'
			}
			return this.value.equals(type);
		}
	}
	
	public static class JSONUndefinedPredicate extends AbstractJSONValuePredicate {
		public JSONUndefinedPredicate(String path) {
			super(path.split("/"));
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			return !value.isPresent();
		}
	}
	
	/**
	 * JSON test op, special case for strings
	 */
	public static class JSONStringTestPredicate extends AbstractJSONValuePredicate {
		protected final String value;
		protected final boolean ignoreCase;
		
		public JSONStringTestPredicate(String path, String value, boolean ignoreCase) {
			super(path.split("/"));
			this.ignoreCase = ignoreCase;
			this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			if (!value.isPresent())
				return false;
			String str = "" + value.get();
			if (ignoreCase)
				return str.equalsIgnoreCase(this.value);
			return str.equals(this.value);
		}
	}
	
	public static class JSONAndPredicate implements JSONPredicate {
		protected final List<JSONPredicate> children;
		
		public JSONAndPredicate(List<JSONPredicate> children) {
			this.children = children;
		}
		
		@Override
		public boolean test(JSONObject object) {
			for (JSONPredicate child : children)
				if (!child.test(object))
					return false;
			return true;
		}
		
		@Override
		public boolean test(JSONArray array) {
			for (JSONPredicate child : children)
				if (!child.test(array))
					return false;
			return true;
		}
		
		@Override
		public boolean test(Map<String, Object> object) {
			for (JSONPredicate child : children)
				if (!child.test(object))
					return false;
			return true;
		}
		
		@Override
		public boolean test(List<Object> array) {
			for (JSONPredicate child : children)
				if (!child.test(array))
					return false;
			return true;
		}
	}
	
	public static class JSONNotPredicate implements JSONPredicate {
		protected final List<JSONPredicate> children;
		
		public JSONNotPredicate(List<JSONPredicate> children) {
			this.children = children;
		}
		
		@Override
		public boolean test(JSONObject object) {
			for (JSONPredicate child : children)
				if (child.test(object))
					return false;
			return true;
		}
		
		@Override
		public boolean test(JSONArray array) {
			for (JSONPredicate child : children)
				if (child.test(array))
					return false;
			return true;
		}
		
		@Override
		public boolean test(Map<String, Object> object) {
			for (JSONPredicate child : children)
				if (child.test(object))
					return false;
			return true;
		}
		
		@Override
		public boolean test(List<Object> array) {
			for (JSONPredicate child : children)
				if (child.test(array))
					return false;
			return true;
		}
	}
	
	public static class JSONOrPredicate implements JSONPredicate {
		protected final List<JSONPredicate> children;
		
		public JSONOrPredicate(List<JSONPredicate> children) {
			this.children = children;
		}
		
		@Override
		public boolean test(JSONObject object) {
			for (JSONPredicate child : children)
				if (child.test(object))
					return true;
			return false;
		}
		
		@Override
		public boolean test(JSONArray array) {
			for (JSONPredicate child : children)
				if (child.test(array))
					return true;
			return false;
		}
		
		@Override
		public boolean test(Map<String, Object> object) {
			for (JSONPredicate child : children)
				if (child.test(object))
					return true;
			return false;
		}
		
		@Override
		public boolean test(List<Object> array) {
			for (JSONPredicate child : children)
				if (child.test(array))
					return true;
			return false;
		}
	}
}