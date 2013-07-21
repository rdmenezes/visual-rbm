
explorer = null;
img_margin = 3;
 
 function IDXExplorer(in_div, in_idx)
 {
	explorer = this;

	this._lead_zeros = Math.floor(Math.log(in_idx._row_count) / Math.LN10);
	console.log("leading zeros: " + this._lead_zeros);
	this._root = in_div;
	this._idx = in_idx;
	
	// create an invisible canvas
	this._canvas = document.createElement("canvas");
	this._canvas.style.display = "none";
	this._context = this._canvas.getContext("2d");
	this._root.appendChild(this._canvas);
	
	// get number of rows
	this._idx_rows = this._idx._row_count;
	// size of images in pixels
	this._image_pixel_width = Math.floor(Math.sqrt(in_idx._row_length));
	this._image_pixel_height = Math.floor(in_idx._row_length / this._image_pixel_width);
	if(this._image_pixel_width * this._image_pixel_height < in_idx._row_length)
	{
		this._image_pixel_height += 1;
	}
	
	// number of images we have per row
	this._row_width = 10;
	
	// data for mous events
	this._mousedown = false;
	this._old_y;
	
	this._client_width = this._root.parentNode.clientWidth;
	this._client_height = this._root.parentNode.clientHeight;
	
	this._tile_width = this._client_width / (this._row_width + 1);
	this._img_width = this._tile_width - 2 * img_margin;
	this._img_height = this._img_width * this._image_pixel_height / this._image_pixel_width;
	this._tile_height = this._img_height + 2 * img_margin;
	
	this._canvas.width = this._img_width;
	this._canvas.height = this._img_height;
	
	this._total_rows = this._idx_rows / this._row_width;
	this._visible_rows = Math.ceil(this._client_height / (this._img_height + img_margin));
	this._rows_loaded = 2 * (this._visible_rows + 1);
	
	this._front_row = Math.floor(this._total_rows - this._rows_loaded / 4);
	this._back_row = (this._front_row + this._rows_loaded - 1) % this._total_rows;

	// current y position of explorer div	
	this._scrollY = (this._total_rows - this._front_row) * -this._tile_height;
	this._root.style.top = this._scrollY + "px";	
	
	this._row_offset = 0;
	
	for(k = 0; k < this._rows_loaded; k++)
	{
		AppendRow(CreateRow((this._front_row + k) % this._total_rows));
	}
	
	in_div.addEventListener("mousedown", function(evt) 
	{
		explorer._mousedown = true;
		explorer._old_y = evt.screenY;
	});
	in_div.addEventListener("mousemove", function(evt)
	{
		if(explorer._mousedown)
		{
			diff = ( explorer._old_y - evt.screenY);
			
			explorer._row_offset -= diff;
			
			explorer._scrollY -= diff;
			explorer._root.style.top = explorer._scrollY + "px";	
			explorer._old_y = evt.screenY;

			// scrolling up
			if(explorer._row_offset <= -explorer._img_height)
			{
				explorer._row_offset += explorer._img_height;

				explorer._front_row = ( explorer._front_row + 1 ) % explorer._total_rows;
				explorer._back_row = ( explorer._back_row + 1 ) % explorer._total_rows;
			
				AppendRow(CreateRow(explorer._back_row));
				RemoveFirstRow();
			}
			else 
			// scrolling down
			if(explorer._row_offset >= explorer._img_height)
			{
				explorer._row_offset -= explorer._img_height;
			
				explorer._front_row = ( explorer._front_row + explorer._total_rows - 1 ) % explorer._total_rows;
				explorer._back_row = ( explorer._back_row + explorer._total_rows- 1 ) % explorer._total_rows;

				PrependRow(CreateRow(explorer._front_row));
				RemoveLastRow();
			}
		}
		
	});
	in_div.addEventListener("mouseup", function(evt) 
	{
		explorer._mousedown = false;
	});
 }
 
 // start_index -> offset into the IDX file to start at
 // count -> the number of images to load
 CreateRow = function(start_index)
 {
	var count = explorer._row_width;
 
	var row_div = document.createElement("div");
	row_div.className = "batch";
	
	div = document.createElement("div");
	div.className = "tile";
	div.style.width = Math.floor(explorer._img_width) + "px";
	div.style.height = div.style.width;
	div.style.fontSize = (Math.floor(explorer._img_width)  * 0.25) + "px";
	row_div.appendChild(div);
	
	span = document.createElement("span");
	span.innerHTML = ZeroPad(start_index, explorer._lead_zeros);
	span.style.lineHeight = div.style.height;
	div.appendChild(span);
	
	
	for(var i = 0; i < count; i++)
	{
		explorer._idx.ReadRow(start_index * explorer._row_width + i,
		function(data, index)
		{
			
			var image_data = explorer._context.createImageData(explorer._canvas.width, explorer._canvas.height);

			// we iterate over our destination canvas, and nearest neighbor sample the source data
			for(var y = 0; y < explorer._canvas.height; y++)
			{
				var yp = Math.floor(y / explorer._canvas.height * explorer._image_pixel_height);
				for(var x = 0; x < explorer._canvas.width; x++)
				{
					var xp = Math.floor(x / explorer._canvas.width * explorer._image_pixel_width);
					
					var pixel_index = yp * explorer._image_pixel_width + xp;
					if(pixel_index < data.length)
					{
						image_data.SetPixel(x, y, data[pixel_index]);
					}
					else
					{
						image_data.SetTransparent(x, y);
					}
					
				}
			}
			// put the image back to the canvas
			explorer._context.putImageData(image_data, 0, 0);
		
			// and export it as png
			img = document.createElement("img");
			img.src = explorer._canvas.toDataURL("image/png");
			img.className = "tile";
			img.draggable = false;

			row_div.replaceChild(img, row_div.childNodes[index % explorer._row_width + 1]);
		});
		
		var temp_div = document.createElement("div");
		temp_div.className = "tile";
		row_div.appendChild(temp_div);
	}
	
	return row_div;
 }
 
 AppendRow = function(row_div)
 {
	explorer._root.appendChild(row_div);
 }
 
 PrependRow = function(row_div)
 {
	// canvas is the first node
	explorer._root.insertBefore(row_div, explorer._root.childNodes[1]);
	
	explorer._scrollY -= explorer._img_height;
	explorer._root.style.top = explorer._scrollY + "px";	
 }
 
 RemoveLastRow = function()
 {
	explorer._root.removeChild(explorer._root.lastChild);
 }
 
 RemoveFirstRow = function()
 {
	// canvas is the first node
 	explorer._root.removeChild(explorer._root.childNodes[1]);
	explorer._scrollY += explorer._img_height;
	explorer._root.style.top = explorer._scrollY + "px";	
}

 // extension method on image data
ImageData.prototype.SetPixel = function(x, y, brightness)
{
	brightness = Math.floor(brightness * 255)
	
	var r = brightness;
	var g = brightness;
	var b = brightness;
	var a = 255;
	
	var index = x * 4 + y * explorer._canvas.width * 4;
	
	this.data[index + 0] = r;
	this.data[index + 1] = g;
	this.data[index + 2] = b;
	this.data[index + 3] = a;
}
 
 ImageData.prototype.SetTransparent = function(x, y, brightness)
{
	var r = 255;
	var g = 0;
	var b = 255;
	var a = 0;
	
	var index = x * 4 + y * explorer._canvas.width * 4;
	
	this.data[index + 0] = r;
	this.data[index + 1] = g;
	this.data[index + 2] = b;
	this.data[index + 3] = a;
}
 
 function ZeroPad(n, p) {
    var pad_char = '0';
    var pad = new Array(1 + p).join(pad_char);
    return (pad + n).slice(-pad.length);
}