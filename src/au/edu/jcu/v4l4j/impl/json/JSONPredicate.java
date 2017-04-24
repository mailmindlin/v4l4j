package au.edu.jcu.v4l4j.impl.json.JSONPredicate;

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
		if (offset == path.length - 1) {
			
		}
		return Optional.empty();
	}
	
	public static Optional<Object> travers(JSONArray array, int offset, String...path) {
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
			return doTest(traverse(object, 0, path));
		}
		
		@Override
		public boolean test(JSONArray array) {
			return doTest(traverse(array, 0, path));
		}
		
		@Override
		public boolean test(Map<String, Object> object) {
			return doTest(traverse(object, 0, path));
		}
		
		@Override
		public boolean test(List<Object> array) {
			return doTest(traverse(array, 0, path));
		}
		
		protected abstract boolean doTest(Optional<Object> value);
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
	
	public static class JSONDefinedPredicate implements AbstractJSONValuePredicate {
		public JSONDefinedPredicate(String path) {
			super(path.split("/"));
		}
		
		@Override
		public boolean test(Optional<Object> value) {
			return value.isPresent();
		}
	}
	
	public static class JSONEndsPredicate implements AbstractJSONValuePredicate {
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
	
	public static class JSONLessPredicate implements AbstractJSONValuePredicate {
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
			return n.doubleValue() < value.doubleValue();
		}
	}
	
	public static class JSONMorePredicate implements AbstractJSONValuePredicate {
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
			return n.doubleValue() > value.doubleValue();
		}
	}
	
	public static class JSONStartsPredicate implements AbstractJSONValuePredicate {
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
	
	public static class JSONTypePredicate implements AbstractJSONValuePredicate {
		protected final String value;
		
		public JSONStartsPredicate(String path, String value) {
			super(path.split("/"));
			this.value = value;
		}
		
		@Override
		public boolean test(Optional<Object> maybe) {
			String type;
			if (!maybe.isPresent())
				type = "undefined";
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
			return type.equals(this.value);
		}
	}
	
	public static class JSONUndefinedPredicate implements AbstractJSONValuePredicate {
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
	public static class JSONStringTestPredicate implements AbstractJSONValuePredicate {
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
				return str.equalsIgnoreCase(value);
			return str.equals(value);
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
		
		public JSONAndPredicate(List<JSONPredicate> children) {
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