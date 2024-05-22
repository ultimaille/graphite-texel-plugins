## Purpose

`graphite-texel-plugins` is a suite of plug-ins offering tools for inspecting and interacting with tet/hex meshes.

## Prerequisites

 - Graphite >= `v3-1.8.8`
 - C++ >= 17
 - Git

To get the plug-in suite up and running, you first need to get and compile Graphite. To do this, please follow the instructions on the official Graphite github: 
https://github.com/BrunoLevy/GraphiteThree. Note that you should follow the instructions for `Compiling sources` whether you're running Windows, Mac or Linux: 

 - https://github.com/BrunoLevy/GraphiteThree/wiki/compiling_Windows
 - https://github.com/BrunoLevy/GraphiteThree/wiki/compiling_Linux

## Installation

Once the graphite installation is complete: 

 1. Go to the `plugins/OGF` directory
 2. Clone this repository `git clone https://github.com/ultimaille/graphite-texel-plugins`
 3. Go to the root directory of Graphite
 4. Re-configure graphite project following your OS instructions
 5. Re-compile graphite !

If everything goes well, you should see the following messages printed on your console after graphite starts up:

```sh
Loading module: Inspect
Loading module: Interactions
```

## Update

The procedure for upgrading is the same as for installation, you just need to replace the step `2` (repository cloning) by a `git pull` command.

## What's inside ?

### Tools

 - Tranform tool: move points in a more friendly way
 - N-Ring: visualize neighborhood of a cell in a volume mesh
 - Extract layer: visualize a hex layer by selecting a cell edge
 - Extract lace: visualize a hex lace by selecting a cell facet


### Commands

 - Conditional cell filter: filter cells by attribute value using a comparator

