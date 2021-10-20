import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PictureMaskComponent } from './picture-mask.component';

describe('PictureMaskComponent', () => {
  let component: PictureMaskComponent;
  let fixture: ComponentFixture<PictureMaskComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PictureMaskComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PictureMaskComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
