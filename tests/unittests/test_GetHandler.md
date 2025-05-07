1. Setup Inputs
    1. Setup Connection 
        - Constructor with some default Values ({} for addr and NULL for prsr) 
        - setState to Handling
 
    2. Setup Request
        - it's a struct, put inside what you need
        
    3. Setup RouteConfig
        - also a struct

2. Setup "Environment"
    1. Setup test_files 
        - files that actually exists and should be read
        - files that don't exist
        - no permission
    
3. Test the changes done by the Handler in the Connection   
    - first and foremost the _response
    - _state (getState)