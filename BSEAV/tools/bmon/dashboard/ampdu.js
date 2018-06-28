/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

        let W = 250, H = 250, D = 250;
        var createScene = function (canvasName, num) {
            // console.log("trying to retrieve " +  "renderCanv" + numbre);
            var canvas = document.getElementById(canvasName);
            var engine = new BABYLON.Engine(canvas, true);
            var scene  = new BABYLON.Scene(engine);
            scene.clearColor = new BABYLON.Color4(1,1,1,1);

            var light = new BABYLON.HemisphericLight("hemi", new BABYLON.Vector3(0, 1, 0), scene);
            // light.intensity = 0.1;
            var camera = new BABYLON.ArcRotateCamera("Camera", 0.5 , 1.5, 490, BABYLON.Vector3.Zero(), scene);
            camera.attachControl(canvas, true);

            //scatterPlot
            var plot = new ScatterPlot(
                //dimensions - width, height, depth
                [W,H,D],
                //labels value - x axis, y axis, z axis
                {
                    x: ["10", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"],
                    y: ["0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50", "55", "60", "65", "70", "75", "80"],
                    z: ["", "VHT3", "VHT2", "VHT1", "VHT0", "SGI3", "SGI2", "SGI1", "SGI0"]
                }, scene, num);

            var vec = [], vec_x = 12, vec_z = 8;
            for (j = 0; j < vec_z; j++) {
                for (i = 0; i < vec_x; i++) {
                    vec.push({x:i, y:0, z:j});
                }
            }

            plot.draw(vec, num, true);

            return { s : scene,
                     e : engine,
                     v : vec,
                     p : plot};
        };


        var ScatterPlot = function (dimensions, labels, scene, num) {
            this.chartnum = num;
            this.scene = scene;
            this.dimensions = {width:200, height:200, depth:200};

            if (dimensions.length>0){
                if(dimensions[0] != undefined)
                    this.dimensions.width = parseFloat(dimensions[0]);
                if(dimensions[1] != undefined)
                    this.dimensions.height = parseFloat(dimensions[1]);
                if(dimensions[2] != undefined)
                    this.dimensions.depth = parseFloat(dimensions[2]);
            }

            this.labelsInfo = {x:[], y:[], z:[]};

            if(Object.keys(labels).length>0){
                if(labels.x != undefined && Array.isArray(labels.x))
                    this.labelsInfo.x = labels.x;
                if(labels.y != undefined && Array.isArray(labels.y))
                    this.labelsInfo.y = labels.y;
                if(labels.z != undefined && Array.isArray(labels.z))
                    this.labelsInfo.z = labels.z;
            }

            this.axis = [];

            //infos for dispose;
            this._materials = [];
            this._meshes = [];
            this._textures = [];

            //the figure
            this.shape = null;

            //the entire scatterPlot
            this.mesh = new BABYLON.Mesh("scatterPlot", this.scene);

            //internals
            this._depth = this.dimensions.depth/2,
            this._width = this.dimensions.width/2,
            this._height = this.dimensions.height/2,
            this._a = this.labelsInfo.y.length,
            this._b = this.labelsInfo.x.length,
            this._c = this.labelsInfo.z.length;
            this._color = new BABYLON.Color3(0.6,0.6,0.6);
            this._defPos = this.mesh.position.clone();

            this._addGrid = function (width, height, linesHeight, linesWidth, position, rotation) {

                var stepw = 2*width/linesWidth,
                steph = 2*height/linesHeight;
                var verts = [];

                //width
                for ( var i = -width; i <= width; i += stepw ) {
                    verts.push([new BABYLON.Vector3( -height, i,0 ), new BABYLON.Vector3( height, i,0 )]);
                }

                //height
                for ( var i = -height; i <= height; i += steph ) {
                    verts.push([new BABYLON.Vector3( i,-width,0 ), new BABYLON.Vector3( i, width, 0 )]);
                }

                this._BBJSaddGrid(verts, position, rotation);
            };

            this._BBJSaddGrid = function (verts, position, rotation){

                var line = BABYLON.MeshBuilder.CreateLineSystem("linesystem", {lines: verts, updatable: false}, this.scene);
                line.color = this._color;

                line.position = position;
                line.rotation = rotation;
                line.parent = this.mesh;
                this.axis.push(line);
                this._meshes.push(line);
            };

            this._addLabel = function (length, data, axis, position) {

                var diff = 2*length/data.length,
                p = new BABYLON.Vector3.Zero(),
                parent = new BABYLON.Mesh("label_"+axis, this.scene);

                for ( var i = 0; i <= data.length; i ++ ) {
                    var label = (i != data.length) ? this._BBJSaddLabel(data[i]) : this._BBJSaddLabel(this.chartnum ? "TX" : "RX");
                    label.position = p.clone();

                    switch(axis.toLowerCase()){
                        case "x":
                            p.subtractInPlace(new BABYLON.Vector3(diff,0,0));
                            break;
                        case "y":
                            p.addInPlace(new BABYLON.Vector3(0, diff, 0));
                            break;
                        case "z":
                            p.subtractInPlace(new BABYLON.Vector3(0,0,diff));
                            break;
                    }
                    label.parent =  parent;
                }
                parent.position = position;
                parent.parent = this.mesh;
                this._meshes.push(parent);
            };

            this._BBJSaddLabel = function(text){

                const planeTexture = new BABYLON.DynamicTexture("dynamic texture", 512, this.scene, true, BABYLON.DynamicTexture.TRILINEAR_SAMPLINGMODE);
                planeTexture.drawText(text, null, null, "normal 200px Helvetica", "black", "transparent", true);

                var material = new BABYLON.StandardMaterial("outputplane", this.scene);
                material.emissiveTexture = planeTexture;
                material.opacityTexture = planeTexture;
                material.backFaceCulling = true;
                material.disableLighting = true;
                material.freeze();

                var outputplane = BABYLON.Mesh.CreatePlane("outputplane", 25, this.scene, false);
                outputplane.billboardMode = BABYLON.AbstractMesh.BILLBOARDMODE_ALL;
                outputplane.material = material;

                this._meshes.push(outputplane);
                this._materials.push(material);
                this._textures.push(planeTexture);

                return outputplane;
            };

            this.setColor = function (color3){
                if(this.axis.length>0){
                    for(var i=0;i<this.axis.length;i++){
                        this.axis[i].color = color3;
                    }
                }
            };

            this.setPosition = function (vector3){
                if(this.mesh){
                    this.mesh.position = vector3;
                }
            };

            this.setScaling = function (vector3){
                if(this.mesh){
                    this.mesh.scaling = vector3;
                }
            };

            this.update = function (vector3_array) {
                this.shape.mesh.dispose();
                /*
                if(vector3_array.length > 0){
                    for(var i=0;i<vector3_array.length;i++){
                        this.points[i].x = vector3_array[i].x*(this.dimensions.width/this._b);
                        this.points[i].y = vector3_array[i].y*(this.dimensions.height/this._a);
                        this.points[i].z = vector3_array[i].z*(this.dimensions.depth/this._c);
                    }
                }
                var p = 0;
                for(var i=0;i<this.points.length;i++){
                    this.shape.particles[p].position.x = this.points[i].x;
                    this.shape.particles[p].position.y = this.points[i].y;
                    this.shape.particles[p].position.z = this.points[i].z;
                     p++;
                }
                */
                // console.log("updating " + vector3_array.length);
            }

            this.draw = function (vector3_array, chartInstance, use_objects = false, gridMaterial = false, convertToFlatShadedMesh = false){
                var points = [];
                if(vector3_array.length > 0){
                    for(var i=0;i<vector3_array.length;i++){
                        points.push(new BABYLON.Vector3(
                            vector3_array[i].x*(this.dimensions.width /this._b),
                            vector3_array[i].y*(this.dimensions.height/this._a),
                            (vector3_array[i].z+1)*(this.dimensions.depth /this._c)
                        ));
                    }
                }

                if(points.length>0){
                    this._defPos = this.mesh.position.clone();
                    this.mesh.position = new BABYLON.Vector3(this._width,this._height,this._depth);

                    if(use_objects){
                        var wdiff = 2*this._width /this.labelsInfo.x.length;
                        var hdiff = 2*this._height/(this.labelsInfo.y.length);
                        var ddiff = 2*this._depth /this.labelsInfo.z.length;
                        var SPS = new BABYLON.SolidParticleSystem('SPS', this.scene);
                        var myMaterial = new BABYLON.StandardMaterial("myMaterial", this.scene);

                        myMaterial.diffuseColor = new BABYLON.Color3(1, 0, 1);
                        myMaterial.specularColor = new BABYLON.Color3(0.5, 0.6, 0.87);
                        myMaterial.emissiveColor = new BABYLON.Color3(1, 1, 1);
                        myMaterial.ambientColor = new BABYLON.Color3(0.23, 0.98, 0.53);
                        myMaterial.alpha = 0.5;

                        SPS.initParticles = function(points,altitude) {
                            var p = 0;
                            for(var i=0;i<points.length;i++){

                                var boxheight = hdiff/5.0 * vector3_array[i].y;
                                var box = BABYLON.MeshBuilder.CreateBox("box", { height: boxheight, width: wdiff, depth: ddiff }, this.scene);
                                box.material = myMaterial;

                                SPS.addShape(box, 1);
                                box.dispose();

                                SPS.particles[p].position.x = points[i].x;
                                SPS.particles[p].position.y = !boxheight ? -2 : boxheight/2 ;
                                SPS.particles[p].position.z = points[i].z;

                                SPS.particles[p].scale.x = 1;
                                SPS.particles[p].scale.y = 1;
                                SPS.particles[p].scale.z = 1;

                                var particleColor = colorAmpduBase.match(/\d+/g);
                                SPS.particles[p].color.r = particleColor[0] / 255.0 + ((!chartInstance) ? vector3_array[i].y / altitude : 0);
                                SPS.particles[p].color.g = particleColor[1] / 255.0 - ((!chartInstance) ? vector3_array[i].y / altitude : 0);
                                SPS.particles[p].color.b = particleColor[2] / 255.0 - ((chartInstance)  ? vector3_array[i].y / altitude : 0);
                                SPS.particles[p].color.a = particleColor[3] / 255.0;

                                // console.log("this.chartnum=" + chartInstance + " fill ampdu r=" + SPS.particles[p].color.r + " g=" + SPS.particles[p].color.g +
                                //    " b=" + SPS.particles[p].color.b + " a=" + SPS.particles[p].color.a);
                                p++;
                            }
                            SPS.buildMesh();
                        };

                        // init all particle values and set them once to apply textures, colors, etc
                        SPS.initParticles(points, this._a);
                        SPS.setParticles();
                        SPS.isAlwaysVisible = true;
                        SPS.computeParticleRotation = false;
                        SPS.computeParticleColor = false;
                        SPS.computeParticleTexture = false;
                        SPS.mesh.position.subtractInPlace(this.mesh.position);
                        SPS.mesh.parent = this.mesh;
                        this._meshes.push(SPS);
                        this.shape = SPS;
                        this.points = points;
                    }
                    else
                    {
                        if(gridMaterial){
                            var mat = new BABYLON.GridMaterial("grid", this.scene);
                            mat.gridRatio = 2;
                            mat.majorUnitFrequency = 1;
                            mat.minorUnitVisibility = 0.1;
                            mat.opacity = 0.98;
                            mat.mainColor = new BABYLON.Color3(1, 1, 1);
                            mat.lineColor = new BABYLON.Color3(0,0,0);
                        }else{
                            var mat = new BABYLON.StandardMaterial("standard", this._scene);
                            mat.specularColor = new BABYLON.Color3();
                        }

                        mat.backFaceCulling = false;

                        var points_chunk = this._chunk(points, this._c-1);

                        var ribbon = BABYLON.Mesh.CreateRibbon("ribbon", points_chunk, false, false, 0,  this.scene, true, BABYLON.Mesh.BACKSIDE);
                        ribbon.material = mat;

                        var vertexData = new BABYLON.VertexData();
                        var pdata = ribbon.getVerticesData(BABYLON.VertexBuffer.PositionKind);

                        var faceColors=[];
                        for(var i=0;i<pdata.length;i+=3)
                        {
                            var coef = (1-pdata[i+1]/this._a/8);
                            faceColors.push(1,coef,0.5,1);
                        }

                        vertexData.colors = faceColors;
                        vertexData.applyToMesh(ribbon, true);
                        ribbon.useVertexColors = true;

                        if(convertToFlatShadedMesh){
                            ribbon.convertToFlatShadedMesh();
                        }

                        ribbon.position.subtractInPlace(this.mesh.position);
                        ribbon.parent = this.mesh;
                        this._meshes.push(ribbon);
                        this._materials.push(mat);
                        this.shape = ribbon;
                    }
                    this.mesh.position = this._defPos;
                }
            };

            this._chunk = function (arr, chunkSize) {
                var R = [];
                for (var i=0,len=arr.length; i<len; i+=chunkSize)
                    R.push(arr.slice(i,i+chunkSize));
                return R;
            };

            this.dispose = function (allmeshes = false){
                if(this.shape!=null){
                    if(this.shape.material != undefined)
                        this.shape.material.dispose();
                    this.shape.dispose();
                    this.shape = null;
                }
                if(allmeshes){
                    if(this._textures.length>0){
                        for(var i=0;i<this._textures.length;i++){
                            this._textures[i].dispose();
                        }
                    }
                    if(this._materials.length>0){
                        for(var i=0;i<this._materials.length;i++){
                            this._materials[i].dispose();
                        }
                    }
                    if(this._meshes.length>0){
                        for(var i=0;i<this._meshes.length;i++){
                            this._meshes[i].dispose();
                        }
                    }
                    if(this.mesh!=null){
                        if(this.mesh.material != null)
                            this.mesh.material.dispose();
                        this.mesh.dispose();
                    }
                    this._meshes = [];
                    this._materials = [];
                    this._textures = [];
                    this.mesh = null;
                    delete this;
                }
            }

            //create items
            this._addGrid(this._height, this._width, this._b, this._a, new BABYLON.Vector3(0,0,-this._depth), BABYLON.Vector3.Zero());
            this._addGrid(this._depth, this._width, this._b, this._c, new BABYLON.Vector3(0,-this._height,0), new BABYLON.Vector3(Math.PI/2,0,0));
            this._addGrid(this._height, this._depth, this._c, this._a, new BABYLON.Vector3(-this._width,0,0), new BABYLON.Vector3(0,Math.PI/2,0));

            this._addLabel(this._width, this.labelsInfo.x, "x", new BABYLON.Vector3(this._width,-(this._height+2),-this._depth));
            this._addLabel(this._height, this.labelsInfo.y, "y", new BABYLON.Vector3(this._width,-this._height,-(this._depth+4)));
            this._addLabel(this._depth, this.labelsInfo.z, "z", new BABYLON.Vector3(this._width+2,-(this._height+2),this._depth));

            return this;
        }

        function loadGraph() {
            sceng0 = createScene("renderCanv0", 0);
            sceng1 = createScene("renderCanv1", 1);

            sceng0.e.runRenderLoop(function () {
                if (sceng0.s) {
                    sceng0.s.render();
                }
            });

            sceng1.e.runRenderLoop(function () {
                if (sceng1.s) {
                    sceng1.s.render();
                }
            });
        }
