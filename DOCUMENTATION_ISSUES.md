# Documentation Issues - Needs Update

## CRITICAL: Documentation is Severely Outdated

Both README.md and QUICKSTART.md describe features and formats that no longer exist in the codebase.

### Issues Found:

#### 1. README.md - Old Pipe Format
**Lines 68-69, and throughout all examples:**

The documentation describes this format:
```
button1+button2+button3|command|description
```

**Current actual format:**
```
*001+002=command    # Trigger format with actions
~001*002=command    # Negate format
```

**Impact:**
- Users following README will create completely non-functional config files
- Examples in lines 72-138 are all invalid
- Troubleshooting section (line 176) incorrectly tells users to use pipes

#### 2. QUICKSTART.md - Non-existent UI
**Lines 34, 71-78:**

Claims there is a "Simple UI - easy configuration window" and describes:
- Opening "Plugins → Multibind" menu
- Clicking "Record" button
- Entering commands in a UI

**Reality:**
- No UI exists in the codebase
- Configuration is file-based only
- Menu only has "Reload Configuration" option

#### 3. Missing Negate Operator Documentation
**All documentation:**

The negate operator (`~`) is a major feature but is:
- Not mentioned in README.md
- Not mentioned in QUICKSTART.md
- Only documented in test files

### Required Actions:

1. **Rewrite README.md** to document:
   - Correct trigger format: `*000+001=command`
   - Action prefixes: `*` (held), `+` (pressed), `-` (released), `~` (NOT held)
   - Negate operator functionality and use cases
   - Remove all pipe format examples
   - Fix all example configurations

2. **Rewrite QUICKSTART.md** to:
   - Remove UI references
   - Document file-based configuration process
   - Show correct configuration file format
   - Add negate operator examples

3. **Create CONFIG_FORMAT.md** with:
   - Detailed syntax specification
   - All operator types
   - Negate operator patterns
   - Valid/invalid examples
   - Best practices

### Current Workaround:

Users can reference the test files for correct syntax:
- `test_negate_config.txt` - Examples of negate operator
- `test_negate_fixed.txt` - Advanced negate patterns

### Priority: HIGH

New users cannot successfully use the plugin with the current documentation.
