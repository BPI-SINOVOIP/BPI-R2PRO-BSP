import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { FaceRoiComponent } from './face-roi.component';

describe('FaceRoiComponent', () => {
  let component: FaceRoiComponent;
  let fixture: ComponentFixture<FaceRoiComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ FaceRoiComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FaceRoiComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
