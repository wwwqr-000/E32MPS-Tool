<h1>E32MPS-Tool</h1>

<h4>A tool to sync your local esp32 files with a esp32 running microPython</h4>


<h5>Recommendations</h5>

* Wait with changing the content of files while the first sync hasn't been made yet.
* Use a base delay of 1 or higher.
* You can terminate E32MPS-Tool or disconnect your esp32 only when the buff.cache file is deleted by the application. This is why you use the delay.
