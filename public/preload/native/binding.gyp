{
  "targets": [
    {
      "target_name": "sysmon",
      "sources": [
        "src/sysmon.cpp",
        "src/network.cpp",
        "src/connections.cpp"
      ],
      "defines": ["NAPI_VERSION=8"],
      "conditions": [
        ["OS=='win'", {
          "libraries": [
            "-lpsapi.lib",
            "-liphlpapi.lib",
            "-lws2_32.lib",
            "-lpdh.lib",
            "-lwbemuuid.lib"
          ]
        }]
      ]
    }
  ]
}
